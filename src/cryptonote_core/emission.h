#pragma once

#include <cstdint>
#include <string>

namespace cryptonote {

  /**
   * RandomLite (XRL) Tokenomics & Block Reward Engine
   */
  class EmissionEngine {
  public:
    // Compute block reward for height given accumulated generated coins
    static uint64_t get_block_reward(uint64_t already_generated_coins, uint64_t current_height);

    // Convert atomic units (10^12) to human-readable XRL string
    static std::string print_money(uint64_t amount);

    // Parse XRL string to atomic units (uint64_t)
    static uint64_t parse_money(const std::string& str);
  };

} // namespace cryptonote
