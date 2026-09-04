#include "blocks.h"

#include <unordered_map>

extern const unsigned char checkpoints[];
extern const size_t checkpoints_len;
extern const unsigned char stagenet_blocks[];
extern const size_t stagenet_blocks_len;
extern const unsigned char testnet_blocks[];
extern const size_t testnet_blocks_len;

namespace blocks
{

  // RandomLite: disable Monero compiled-in fast-sync hashes (wrong genesis)
  const epee::span<const unsigned char> GetCheckpointsData(cryptonote::network_type network)
  {
    (void)network;
    return {};
  }

}
