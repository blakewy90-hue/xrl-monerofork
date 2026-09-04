#pragma once

#include <cstdint>
#include <vector>

namespace crypto {

  typedef uint64_t difficulty_type;

  /**
   * Zawy LWMA-3 Difficulty Adjustment Algorithm for RandomLite (XRL)
   * 
   * @param timestamps Vector of timestamps for the recent N blocks
   * @param cumulative_difficulties Cumulative difficulty vector
   * @param target_seconds Block target time in seconds (150s)
   * @param window_size Number of blocks in sliding window (720 blocks)
   * @return Next difficulty target
   */
  difficulty_type next_difficulty_lwma3(
      std::vector<uint64_t> timestamps,
      std::vector<difficulty_type> cumulative_difficulties,
      uint64_t target_seconds,
      size_t window_size
  );

  /**
   * Check if Proof-of-Work hash meets target difficulty
   */
  bool check_pow_hash(const uint8_t pow_hash[32], difficulty_type target_difficulty);

} // namespace crypto
