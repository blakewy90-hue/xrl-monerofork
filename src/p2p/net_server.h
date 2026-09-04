#pragma once

#include "../cryptonote_config.h"
#include <string>
#include <vector>
#include <thread>
#include <atomic>

namespace p2p {

  struct PeerNode {
    std::string ip;
    uint16_t port;
    bool is_connected;
    uint64_t last_seen;
  };

  class P2PServer {
  public:
    P2PServer() = default;
    ~P2PServer() = default;

    // Start P2P TCP Listener on port 28080
    bool start(uint16_t port = P2P_DEFAULT_PORT);

    // Connect to network seed nodes
    void connect_seed_nodes();

    // Broadcast new block header to connected peers
    void broadcast_block(const uint8_t block_hash[32], uint64_t height);

    // Stop P2P Server
    void stop();

    // Get count of connected peer nodes
    size_t get_peer_count() const { return m_peers.size(); }

  private:
    uint16_t m_port = P2P_DEFAULT_PORT;
    std::atomic<bool> m_running{false};
    std::vector<PeerNode> m_peers;
    std::thread m_listener_thread;
  };

} // namespace p2p
