#include "emission.h"
#include "../cryptonote_config.h"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace cryptonote {

  // Maximum base supply = 18.4 Million XRL (in atomic units 10^12)
  static const uint64_t BASE_SUPPLY = UINT64_C(18400000) * COIN_UNITS;
  // Tail emission = 0.6 XRL per 150s block
  static const uint64_t TAIL_EMISSION_REWARD = UINT64_C(600000000000); // 0.6 XRL

  uint64_t EmissionEngine::get_block_reward(uint64_t already_generated_coins, uint64_t current_height) {
    if (current_height == 0) {
      // Genesis Block Premine / Initial Alloc (0 XRL or minimal Genesis output)
      return UINT64_C(100) * COIN_UNITS; 
    }

    if (already_generated_coins >= BASE_SUPPLY) {
      // Base supply reached -> Perpetual Tail Emission
      return TAIL_EMISSION_REWARD;
    }

    // Smooth exponential decay formula: (BASE_SUPPLY - already_generated) >> 20
    uint64_t base_reward = (BASE_SUPPLY - already_generated_coins) >> EMISSION_SPEED_FACTOR_PER_MINUTE;

    // Enforce tail emission floor
    if (base_reward < TAIL_EMISSION_REWARD) {
      base_reward = TAIL_EMISSION_REWARD;
    }

    return base_reward;
  }

  std::string EmissionEngine::print_money(uint64_t amount) {
    uint64_t coins = amount / COIN_UNITS;
    uint64_t fraction = amount % COIN_UNITS;

    std::stringstream ss;
    ss << coins << "." << std::setw(12) << std::setfill('0') << fraction << " XRL";
    return ss.str();
  }

  uint64_t EmissionEngine::parse_money(const std::string& str) {
    double val = std::stod(str);
    return static_cast<uint64_t>(val * static_cast<double>(COIN_UNITS));
  }

} // namespace cryptonote
