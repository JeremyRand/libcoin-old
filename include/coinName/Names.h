/* -*-c++-*- libcoin - Copyright (C) 2014 Daniel Kraft
 *
 * libcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * libcoin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libcoin.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COINNAME_NAMES_H
#define COINNAME_NAMES_H

#include <coinChain/BlockChain.h>
#include <coinChain/Spendables.h>

#include <coinName/Export.h>

#include <string>

/* ************************************************************************** */
/* NameDbRow.  */

/**
 * Struct to hold data about one name history entry, corresponding to
 * the Names database table.
 */
struct COINNAME_EXPORT NameDbRow
{

  bool found;

  int64_t coin;
  int64_t count;
  Evaluator::Value name;
  Evaluator::Value value;

  inline NameDbRow ()
    : found(false)
  {}

  inline NameDbRow (int64_t co, int64_t cnt, const Evaluator::Value& n,
                    const Evaluator::Value& v)
    : found(true), coin(co), count(cnt), name(n), value(v)
  {}

};

/* ************************************************************************** */
/* Name base class.  */

/**
 * Represent information about a name as per name_show.  This uses the NameDbRow
 * as back-end data, but allows further information retrieval.
 */
class COINNAME_EXPORT NameStatus
{

private:

  /** Data from the DB.  */
  NameDbRow data;

  /**
   * The Unspent data (txid, script) associated with the name's coin.  If this
   * has already been purged from the DB, may not be valid.
   */
  Unspent coin;

  /** Blockchain backing this.  */
  const BlockChain& chain;

public:

  /**
   * Construct it with the given data.
   * @param d Data from the DB to use.
   * @param c Blockchain object.
   */
  NameStatus (const NameDbRow& d, const BlockChain& c);

  /**
   * Retrieve the name's name.
   * @return The name's "name" as string.
   */
  inline std::string
  getName () const
  {
    return std::string (data.name.begin (), data.name.end ());
  }

  /**
   * Retrieve the name's value.
   * @return The name's "value" as string.
   */
  inline std::string
  getValue () const
  {
    return std::string (data.value.begin (), data.value.end ());
  }

  /**
   * Get expire counter (as in "expires_in").
   * @return The name's expire counter.
   */
  inline int
  getExpireCounter () const
  {
    return data.count - chain.getExpirationCount ();
  }

  /**
   * Check if the name is expired.
   * @return True iff the name is expired.
   */
  inline bool
  isExpired () const
  {
    return chain.isExpired (data.count);
  }

  /**
   * Try to get the txid associated to this name.  Returns "" if it is not
   * available (purged already).
   * @return This name's txid.
   */
  std::string getTransactionId () const;

  /**
   * Try to get the address that is holding the name.  May return "" if either
   * the name's spending is already purged from the database or if the
   * script can not be parsed.
   * @return This name's address.
   */
  std::string getAddress () const;

};

#endif // NAMES_H
