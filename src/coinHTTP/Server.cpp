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

#include "coinHTTP/Server.h"
#include <boost/bind.hpp>
#include <signal.h>

using namespace std;
using namespace boost;
using namespace asio;

Server::Server(const string address, const string port, const string doc_root) : _io_service(), _context(boost::asio::ssl::context::sslv23), _signals(_io_service), _acceptor(_io_service), _connectionManager(), _new_connection(), _requestHandler(doc_root), _secure(false) {
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
	if(doc_root == "") _requestHandler.setDocRoot(filesystem::initial_path().string());
    _signals.add(SIGINT);
    _signals.add(SIGTERM);
#if defined(SIGQUIT)
    _signals.add(SIGQUIT);
#endif // defined(SIGQUIT)
    _signals.async_wait(bind(&Server::handle_stop, this));
    
    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    ip::tcp::resolver resolver(_io_service);
    ip::tcp::resolver::query query(address, port);
    ip::tcp::endpoint endpoint = *resolver.resolve(query);
    _acceptor.open(endpoint.protocol());
    _acceptor.set_option(ip::tcp::acceptor::reuse_address(true));
    _acceptor.bind(endpoint);
    _acceptor.listen();    
}

void Server::setCredentials(const std::string dataDir, const std::string cert, const std::string key) {
    _context.set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2);
    filesystem::path certfile = cert;
    if (!certfile.is_complete())
        certfile = filesystem::path(dataDir) / certfile;
    if (filesystem::exists(certfile))
        _context.use_certificate_chain_file(certfile.string().c_str());
    else
        throw runtime_error(("SecureServer ERROR: missing server certificate file " + certfile.string()).c_str());
    
    filesystem::path keyfile = key;
    if (!keyfile.is_complete())
        keyfile = filesystem::path(dataDir) / keyfile;
    if (filesystem::exists(keyfile))
        _context.use_private_key_file(keyfile.string().c_str(), ssl::context::pem);
    else
        throw runtime_error(("SecureServer ERROR: missing server private key file " + keyfile.string()).c_str());
    _secure = true;
}

void Server::run() {
    // The io_service::run() call will block until all asynchronous operations
    // have finished. While the server is running, there is always at least one
    // asynchronous operation outstanding: the asynchronous accept call waiting
    // for new incoming connections.
    start_accept();
    _io_service.run();
}

void Server::shutdown(){
    _io_service.post(bind(&Server::handle_stop, this));
}

void Server::start_accept() {
    if(isSecure())
        _new_connection.reset(new Connection(_io_service, _context, _connectionManager, _requestHandler));
    else
        _new_connection.reset(new Connection(_io_service, _connectionManager, _requestHandler));
        
    _acceptor.async_accept(_new_connection->socket(), bind(&Server::handle_accept, this, placeholders::error));
}

void Server::handle_accept(const system::error_code& e) {
    // Check whether the server was stopped by a signal before this completion
    // handler had a chance to run.
    if (!_acceptor.is_open()) {
        return;
    }
    
    if (!e) {
        _connectionManager.start(_new_connection);
    }
    
    start_accept();
}

void Server::handle_stop() {
    // The server is stopped by cancelling all outstanding asynchronous
    // operations. Once all operations have finished the io_service::run() call
    // will exit.
    _acceptor.close();
    _connectionManager.stop_all();
    _io_service.stop();
}
