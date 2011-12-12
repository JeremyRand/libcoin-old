
#ifndef BLOCK_H
#define BLOCK_H

//#include "btc/bignum.h"
//#include "btc/key.h"
//#include "btc/script.h"
//#include "btc/tx.h"

//#include "btcNode/main.h"

#include "btc/uint256.h"

#include <vector>

class Block;
class CBlockIndex;

class CKeyItem;

class Endpoint;
class Inventory;
class CRequestTracker;
class CNode;
class CBlockIndex;

class CTransaction;

class CTxDB;
class TxIndex;


//
// Nodes collect new transactions into a block, hash them into a hash tree,
// and scan through nonce values to make the block's hash satisfy proof-of-work
// requirements.  When they solve the proof-of-work, they broadcast the block
// to everyone and the block is added to the block chain.  The first transaction
// in the block is a special one that creates a new coin owned by the creator
// of the block.
//
// Blocks are appended to blk0001.dat files on disk.  Their location on disk
// is indexed by CBlockIndex objects in memory.
//

typedef std::vector<CTransaction> TransactionList;
typedef std::vector<uint256> MerkleBranch;

class Block
{
public:
    Block()
    {
        setNull();
    }

    Block(const int version, const uint256 prevBlock, const uint256 merkleRoot, const int time, const int bits, const int nonce) : _version(version), _prevBlock(prevBlock), _merkleRoot(merkleRoot), _time(time), _bits(bits), _nonce(nonce)
    {
        _transactions.clear();
        _merkleTree.clear();
    }

    
    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->_version);
        nVersion = this->_version;
        READWRITE(_prevBlock);
        READWRITE(_merkleRoot);
        READWRITE(_time);
        READWRITE(_bits);
        READWRITE(_nonce);

        // ConnectBlock depends on vtx being last so it can calculate offset
        if (!(nType & (SER_GETHASH|SER_BLOCKHEADERONLY)))
            READWRITE(_transactions);
        else if (fRead)
            const_cast<Block*>(this)->_transactions.clear();
    )

    void setNull()
    {
        _version = 1;
        _prevBlock = 0;
        _merkleRoot = 0;
        _time = 0;
        _bits = 0;
        _nonce = 0;
        _transactions.clear();
        _merkleTree.clear();
    }

    bool isNull() const
    {
        return (_bits == 0);
    }

    uint256 getHash() const;

    int64 getBlockTime() const
    {
        return (int64)_time;
    }

    int GetSigOpCount() const;

    void addTransaction(const CTransaction& tx) { _transactions.push_back(tx); }
    size_t getNumTransactions() { return _transactions.size(); }
    const TransactionList getTransactions() const { return _transactions; }
    
    uint256 buildMerkleTree() const;

    MerkleBranch getMerkleBranch(int index) const;

    static uint256 checkMerkleBranch(uint256 hash, const std::vector<uint256>& merkleBranch, int index);

    void print() const;

    bool disconnectBlock(CTxDB& txdb, CBlockIndex* pindex);
    bool connectBlock(CTxDB& txdb, CBlockIndex* pindex);
    bool setBestChain(CTxDB& txdb, CBlockIndex* pindexNew);
    bool addToBlockIndex(unsigned int nFile, unsigned int nBlockPos);
    bool checkBlock() const;
    bool acceptBlock();
    
    const int getVersion() const { return _version; }
    
    const uint256 getPrevBlock() const { return _prevBlock; }

    const uint256 getMerkleRoot() const { return _merkleRoot; }

    const int getTime() const { return _time; }
    
    const int getBits() const { return _bits; }
    
    const int getNonce() const { return _nonce; }
    
    
private:
    // header
    int _version;
    uint256 _prevBlock;
    uint256 _merkleRoot;
    unsigned int _time;
    unsigned int _bits;
    unsigned int _nonce;
    
    // network and disk
    TransactionList _transactions;
    
    // memory only
    mutable MerkleBranch _merkleTree;
};

#endif // BLOCK_H
