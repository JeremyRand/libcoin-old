/* -*-c++-*- libcoin - Copyright (C) 2012 Michael Gronager
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

#include "coinChain/MessageHandler.h"
#include "coin/serialize.h"

#include <string>

using namespace std;
using namespace boost;

MessageHandler::MessageHandler() {
}

void MessageHandler::installFilter(filter_ptr filter) {
        _filters.push_back(filter);
}

bool MessageHandler::handleMessage(Peer* origin, Message& msg) {

    try {
        bool ret = false;
        for(Filters::iterator filter = _filters.begin(); filter != _filters.end(); ++filter) {
            if ((*filter)->commands().count(msg.command())) {
                // copy the string
                string payload(msg.payload());
                //                Message message(msg.command(), payload);
                Message message(msg);
                // We need only one successfull command to return true
                if ( (**filter)(origin, message) ) ret = true;
            }
        }
        return ret;
    } catch (OriginNotReady e) { // Must have a version message before anything else
        return false;        
    }
}
