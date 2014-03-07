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


#include <coinName/NameGetRPC.h>

#include <coinChain/NodeRPC.h>

#include <coinHTTP/Server.h>
#include <coinHTTP/RPC.h>

json_spirit::Value
NameShow::operator() (const json_spirit::Array& params, bool fHelp)
{
  if (fHelp || params.size () != 1)
    throw RPC::error (RPC::invalid_params,
                      "name_show <name>\n"
                      "Show information about <name>.");

  /* FIXME: Correctly handle integer parameters (convert to string).  */
  const std::string name = params[0].get_str ();

  json_spirit::Object res;
  res.push_back (json_spirit::Pair ("name", name));

  return res;
}
