#include "rpc_server.h"
#include "../cryptonote_core/emission.h"
#include <iostream>
#include <sstream>
#include <chrono>

namespace rpc {

  bool RPCServer::start(cryptonote::Blockchain *chain, uint16_t port) {
    m_chain = chain;
    m_port = port;
    m_running = true;

    std::cout << "[RPC Server] Listening for HTTP JSON-RPC calls on http://127.0.0.1:" << m_port << "/json_rpc" << std::endl;

    m_rpc_thread = std::thread([this]() {
      while (m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        // HTTP JSON-RPC Request Listener
      }
    });

    return true;
  }

  std::string RPCServer::handle_request(const std::string &method, const std::string &params_json) {
    if (!m_chain) return "{\"error\": \"Blockchain state uninitialized\"}";

    if (method == "get_info") {
      std::stringstream ss;
      ss << "{\n"
         << "  \"height\": " << m_chain->get_current_height() << ",\n"
         << "  \"difficulty\": " << m_chain->get_current_difficulty() << ",\n"
         << "  \"target_time\": " << DIFFICULTY_TARGET_V2 << ",\n"
         << "  \"status\": \"OK\",\n"
         << "  \"coin_name\": \"" << COIN_NAME << "\",\n"
         << "  \"ticker\": \"" << COIN_TICKER << "\"\n"
         << "}";
      return ss.str();
    } else if (method == "get_block_template") {
      std::stringstream ss;
      ss << "{\n"
         << "  \"height\": " << m_chain->get_current_height() + 1 << ",\n"
         << "  \"difficulty\": " << m_chain->get_current_difficulty() << ",\n"
         << "  \"reserved_offset\": 40,\n"
         << "  \"status\": \"OK\"\n"
         << "}";
      return ss.str();
    } else if (method == "submit_block") {
      return "{\"error\": \"Block submission is unavailable until canonical block parsing and validation are implemented\"}";
    }

    return "{\"error\": \"Unknown RPC method\"}";
  }

  void RPCServer::stop() {
    m_running = false;
    if (m_rpc_thread.joinable()) {
      m_rpc_thread.join();
    }
    std::cout << "[RPC Server] Shutdown cleanly." << std::endl;
  }

} // namespace rpc
