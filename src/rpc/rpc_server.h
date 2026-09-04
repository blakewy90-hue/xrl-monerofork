#pragma once

#include "../cryptonote_config.h"
#include "../cryptonote_core/blockchain.h"
#include <string>
#include <thread>
#include <atomic>

namespace rpc {

  class RPCServer {
  public:
    RPCServer() = default;
    ~RPCServer() = default;

    // Start HTTP JSON-RPC Server on port 28081
    bool start(cryptonote::Blockchain *chain, uint16_t port = RPC_DEFAULT_PORT);

    // Handle JSON-RPC method request
    std::string handle_request(const std::string &method, const std::string &params_json);

    // Stop RPC Server
    void stop();

  private:
    uint16_t m_port = RPC_DEFAULT_PORT;
    cryptonote::Blockchain *m_chain = nullptr;
    std::atomic<bool> m_running{false};
    std::thread m_rpc_thread;
  };

} // namespace rpc
