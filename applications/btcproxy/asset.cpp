// Copyright (c) 2011 Michael Gronager
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#include "asset.h"

#include "btcNode/db.h"

using namespace std;
using namespace boost;

bool Solver(const CScript& scriptPubKey, vector<pair<opcodetype, vector<unsigned char> > >& vSolutionRet);

set<uint160> CAsset::getAddresses()
{
    set<uint160> addresses;
    for(KeyMap::iterator addr = _keymap.begin(); addr != _keymap.end(); ++addr)
        addresses.insert(addr->first);
    return addresses;
}
    
bool CAsset::AddKey(const CKey& key)
{
    _keymap[Hash160(key.GetPubKey())] = key;
    return true;
}

bool CAsset::HaveKey(const CBitcoinAddress &address) const
{
    uint160 hash160 = address.GetHash160();
    return (_keymap.count(hash160) > 0);
}

bool CAsset::GetKey(const CBitcoinAddress &address, CKey& keyOut) const
{
    uint160 hash160 = address.GetHash160();
    KeyMap::const_iterator pair = _keymap.find(hash160);
    if(pair != _keymap.end())
    {
        keyOut = pair->second;
        return true;
    }
    return false;        
}

const CTransaction& CAsset::getTx(uint256 hash) const
{
    TxCache::const_iterator pair = _tx_cache.find(hash);
    if(pair != _tx_cache.end())
    {
        return pair->second;
    }
    //        cout << "CACHE MISS!!! " << hash.ToString() << endl;
    // throw something!
}

uint160 CAsset::getAddress(const CTxOut& out)
{
    vector<pair<opcodetype, vector<unsigned char> > > vSolution;
    if (!Solver(out.scriptPubKey, vSolution))
        return 0;
    
    BOOST_FOREACH(PAIRTYPE(opcodetype, vector<unsigned char>)& item, vSolution)
    {
        vector<unsigned char> vchPubKey;
        if (item.first == OP_PUBKEY)
        {
            // encode the pubkey into a hash160
            return Hash160(item.second);                
        }
        else if (item.first == OP_PUBKEYHASH)
        {
            return uint160(item.second);                
        }
    }
    return 0;
}

void CAsset::syncronize()
{
    CTxDB txdb("r");
    
    _coins.clear();
    
    // read all relevant tx'es
    for(KeyMap::iterator key = _keymap.begin(); key != _keymap.end(); ++key)
    {
        Coins debit;
        txdb.ReadDrIndex(key->first, debit);
        Coins credit;
        txdb.ReadCrIndex(key->first, credit);
        
        Coins coins;
        for(Coins::iterator coin = debit.begin(); coin != debit.end(); ++coin)
        {
            CTransaction tx;
            txdb.ReadDiskTx(coin->first, tx);
            _tx_cache[coin->first] = tx; // caches the relevant transactions
            coins.insert(*coin);
        }
        for(Coins::iterator coin = credit.begin(); coin != credit.end(); ++coin)
        {
            CTransaction tx;
            txdb.ReadDiskTx(coin->first, tx);
            _tx_cache[coin->first] = tx; // caches the relevant transactions
            
            CTxIn in = tx.vin[coin->second];
            Coin spend(in.prevout.hash, in.prevout.n);
            coins.erase(spend);
        }
        
        _coins.insert(coins.begin(), coins.end());
    }
    
    // now _coins contains all non-spend coins !
}

bool CAsset::isSpendable(Coin coin)
{
    // get the address
    CTransaction tx = getTx(coin.first);
    CTxOut& out = tx.vout[coin.second];
    uint160 addr = getAddress(out);
    KeyMap::iterator keypair = _keymap.find(addr);
    if(keypair->second.IsNull())
        return false;
    
    try {
        CPrivKey privkey = keypair->second.GetPrivKey();
    }
    catch(key_error err)
    {
        return false;
    }
    return true;        
}

const int64 CAsset::value(Coin coin) const
{
    const CTransaction& tx = getTx(coin.first);
    const CTxOut& out = tx.vout[coin.second];
    return out.nValue;
}

int64 CAsset::balance()
{
    int64 sum = 0;
    for(Coins::iterator coin = _coins.begin(); coin != _coins.end(); ++coin)
        sum += value(*coin);
    return sum;
}

int64 CAsset::spendable_balance()
{
    int64 sum = 0;
    for(Coins::iterator coin = _coins.begin(); coin != _coins.end(); ++coin)
        if(isSpendable(*coin))
            sum += value(*coin);
    return sum;
}

CTransaction CAsset::generateTx(set<Payment> payments, uint160 changeaddr)
{
    int64 amount = 0;
    for(set<Payment>::iterator payment = payments.begin(); payment != payments.end(); ++payment)
        amount += payment->second;
    
    // calculate fee
    int64 fee = max(amount/1000, (int64)500000);
    amount += fee;
    
    // create a list of spendable coins
    vector<Coin> spendable_coins;
    for(Coins::iterator coin = _coins.begin(); coin != _coins.end(); ++coin)
        if (isSpendable(*coin)) {
            spendable_coins.push_back(*coin);
        }
    
    CompValue compvalue(*this);
    sort(spendable_coins.begin(), spendable_coins.end(), compvalue);
    
    int64 sum = 0;
    int coins = 0;
    for(coins = 0; coins < spendable_coins.size(); coins++)
    {
        sum += value(spendable_coins[coins]);
        if(sum > amount)
            break;
    }
    
    CTransaction tx;
    
    if(sum < amount) // could not pay - throw something
        return tx;
    
    // shuffle the inputs
    random_shuffle(spendable_coins.begin(), spendable_coins.end());
    
    // now fill in the tx outs
    for(set<Payment>::iterator payment = payments.begin(); payment != payments.end(); ++payment)
    {
        CScript scriptPubKey;
        scriptPubKey << OP_DUP << OP_HASH160 << payment->first << OP_EQUALVERIFY << OP_CHECKSIG;
        
        CTxOut out(payment->second, scriptPubKey);
        tx.vout.push_back(out);
    }            
    // handle change!
    int64 change = amount - sum;
    uint160 changeto = changeaddr;
    if(changeto == 0)
    {
        CTransaction txchange = getTx(spendable_coins[0].first);
        uint160 changeto = getAddress(txchange.vout[spendable_coins[0].second]);
    }
    
    if(change > 50000) // skip smaller amounts of change
    {
        CScript scriptPubKey;
        scriptPubKey << OP_DUP << OP_HASH160 << changeto << OP_EQUALVERIFY << OP_CHECKSIG;
        
        CTxOut out(change, scriptPubKey);
        tx.vout.push_back(out);
    }            
    
    int64 invalue = 0;
    for(int i = 0; i < coins; i++)
    {
        const Coin& coin = spendable_coins[i];
        invalue += value(spendable_coins[i]);
        CTxIn in(coin.first, coin.second);
        
        tx.vin.push_back(in);
    }
    
    for(int i = 0; i < coins; i++)
    {
        const Coin& coin = spendable_coins[i];
        const CTransaction& txfrom = getTx(coin.first);
        if (!SignSignature(*this, txfrom, tx, i))
        {
            // throw something!                
        }
    }        
    return tx;
}

CTransaction CAsset::generateTx(uint160 to, int64 amount, uint160 change)
{
    set<Payment> payments;
    payments.insert(Payment(to, amount));
    return generateTx(payments, change);
}
