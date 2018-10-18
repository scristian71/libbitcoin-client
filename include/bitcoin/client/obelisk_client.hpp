/**
 * Copyright (c) 2011-2017 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_CLIENT_OBELISK_CLIENT_HPP
#define LIBBITCOIN_CLIENT_OBELISK_CLIENT_HPP

#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client/define.hpp>
#include <bitcoin/client/history.hpp>
#include <bitcoin/client/stealth.hpp>
#include <bitcoin/protocol.hpp>

namespace libbitcoin {
namespace client {

/// Structure used for passing connection settings for a server.
struct BCC_API connection_settings
{
    int32_t retries;
    config::endpoint server;
    config::endpoint block_server;
    config::endpoint transaction_server;
    config::authority socks;
    config::sodium server_public_key;
    config::sodium client_private_key;
};

/// Client implements a router-dealer interface to communicate with
/// the server over either public or secure sockets.
class BCC_API obelisk_client
{
public:
    typedef std::function<void(const std::string&, uint32_t,
        const data_chunk&)> command_handler;
    typedef std::unordered_map<std::string, command_handler> command_map;

    // Subscription/notification handler types.
    //-------------------------------------------------------------------------

    typedef std::function<void(const code&, uint16_t, size_t,
        const hash_digest&)> update_handler;
    typedef std::function<void(const chain::block&)> block_update_handler;
    typedef std::function<void(const chain::transaction&)>
        transaction_update_handler;

    // Fetch handler types.
    //-------------------------------------------------------------------------

    typedef std::function<void(const code&)> result_handler;
    typedef std::function<void(const code&, size_t)> height_handler;
    typedef std::function<void(const code&, size_t, size_t)> transaction_index_handler;
    typedef std::function<void(const code&, const chain::block&)> block_handler;
    typedef std::function<void(const code&, const chain::header&)> block_header_handler;
    typedef std::function<void(const code&, const chain::transaction&)> transaction_handler;
    typedef std::function<void(const code&, const chain::points_value&)> points_value_handler;
    typedef std::function<void(const code&, const client::history::list&)> history_handler;
    typedef std::function<void(const code&, const client::stealth::list&)> stealth_handler;

    // Used for mapping specific requests to specific handlers
    // (allowing support for different handlers for different client
    // API calls on a per-client instance basis).
    typedef std::unordered_map<uint32_t, result_handler> result_handler_map;
    typedef std::unordered_map<uint32_t, height_handler> height_handler_map;
    typedef std::unordered_map<uint32_t, transaction_index_handler> transaction_index_handler_map;
    typedef std::unordered_map<uint32_t, block_handler> block_handler_map;
    typedef std::unordered_map<uint32_t, block_header_handler> block_header_handler_map;
    typedef std::unordered_map<uint32_t, transaction_handler> transaction_handler_map;
    typedef std::unordered_map<uint32_t, history_handler> history_handler_map;
    typedef std::unordered_map<uint32_t, stealth_handler> stealth_handler_map;
    typedef std::unordered_map<uint32_t, update_handler> update_handler_map;

    /// Construct an instance of the client.
    obelisk_client(int32_t retries=5);

    ~obelisk_client();

    /// Connect to the specified endpoint using the provided keys.
    bool connect(const config::endpoint& address,
        const config::authority& socks_proxy,
        const config::sodium& server_public_key,
        const config::sodium& client_private_key);

    /// Connect to the specified endpoint.
    bool connect(const config::endpoint& address);

    /// Connect using the provided settings.
    bool connect(const connection_settings& settings);

    /// Wait for server to respond to queries, until timeout.
    void wait(size_t timeout_milliseconds=30000);

    /// Monitor for subscription notifications, until timeout.
    void monitor(size_t timeout_milliseconds=30000);

    // Fetchers.
    //-------------------------------------------------------------------------

    void transaction_pool_broadcast(result_handler handler,
        const chain::transaction& tx);

    void transaction_pool_validate2(result_handler handler,
        const chain::transaction& tx);

    void transaction_pool_fetch_transaction(transaction_handler handler,
        const hash_digest& tx_hash);

    void transaction_pool_fetch_transaction2(transaction_handler handler,
        const hash_digest& tx_hash);

    void blockchain_broadcast(result_handler handler,
        const chain::block& block);

    void blockchain_validate(result_handler handler, const chain::block& block);

    void blockchain_fetch_transaction(transaction_handler handler,
        const hash_digest& tx_hash);

    void blockchain_fetch_transaction2(transaction_handler handler,
        const hash_digest& tx_hash);

    void blockchain_fetch_last_height(height_handler handler);

    void blockchain_fetch_block(block_handler handler, uint32_t height);

    void blockchain_fetch_block(block_handler handler,
        const hash_digest& block_hash);

    void blockchain_fetch_block_header(block_header_handler handler,
        uint32_t height);

    void blockchain_fetch_block_header(block_header_handler handler,
        const hash_digest& block_hash);

    void blockchain_fetch_transaction_index(transaction_index_handler handler,
        const hash_digest& tx_hash);

    void blockchain_fetch_stealth2(stealth_handler handler,
        const binary& prefix, uint32_t from_height=0);

    void blockchain_fetch_history4(history_handler handler,
        const wallet::payment_address& address, uint32_t from_height=0);

    void blockchain_fetch_unspent_outputs(points_value_handler handler,
        const wallet::payment_address& address, uint64_t satoshi,
        wallet::select_outputs::algorithm algorithm);

    // Subscribers.
    //-------------------------------------------------------------------------

    void subscribe_address(update_handler handler,
        const short_hash& address_hash);

    void subscribe_stealth(update_handler handler,
        const binary& stealth_prefix);

    bool subscribe_block(const config::endpoint& address,
        block_update_handler on_update);

    bool subscribe_transaction(const config::endpoint& address,
        transaction_update_handler on_update);

private:
    // Attach handlers for all supported client-server operations.
    void attach_handlers();
    void handle_immediate(const std::string& command, uint32_t id,
        const code& ec);

    // Determines if any requests have not been handled.
    bool requests_outstanding();

    // Calls all remaining/expired handlers with the specified error.
    void clear_outstanding_requests(const bc::code& ec);

    // Sends an outgoing request via the internal router.
    bool send_request(const std::string& command, uint32_t id,
        const data_chunk& payload);

    protocol::zmq::context context_;

    // Sockets that connect to external libbitcoin services.
    protocol::zmq::socket socket_;
    protocol::zmq::socket block_socket_;
    protocol::zmq::socket transaction_socket_;

    // Internal socket pair for client request forwarding to router
    // (that then forwards to server).
    protocol::zmq::socket dealer_;
    protocol::zmq::socket router_;

    block_update_handler on_block_update_;
    transaction_update_handler on_transaction_update_;
    int32_t retries_;
    bool secure_;
    config::endpoint worker_;
    uint32_t last_request_index_;
    command_map command_handlers_;
    result_handler_map result_handlers_;
    height_handler_map height_handlers_;
    transaction_index_handler_map transaction_index_handlers_;
    block_handler_map block_handlers_;
    block_header_handler_map block_header_handlers_;
    transaction_handler_map transaction_handlers_;
    history_handler_map history_handlers_;
    stealth_handler_map stealth_handlers_;
    update_handler_map update_handlers_;
};

} // namespace client
} // namespace libbitcoin

#endif
