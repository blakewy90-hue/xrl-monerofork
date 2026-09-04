#include "difficulty.h"
#include "../cryptonote_config.h"
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <cmath>

namespace crypto {

  difficulty_type next_difficulty_lwma3(
      std::vector<uint64_t> timestamps,
      std::vector<difficulty_type> cumulative_difficulties,
      uint64_t target_seconds,
      size_t window_size
  ) {
    size_t count = timestamps.size();
    if (count < 2 || cumulative_difficulties.size() < 2) {
      return 1; // Initial bootstrapping difficulty for Block #1
    }

    // Dynamic sliding window up to 100 blocks
    size_t n = std::min<size_t>(count - 1, 100);
    
    uint64_t lwma_sum = 0;
    uint64_t previous_timestamp = timestamps[count - n - 1];

    for (size_t i = 1; i <= n; ++i) {
      uint64_t current_timestamp = timestamps[count - n + i - 1];
      int64_t solve_time = static_cast<int64_t>(current_timestamp) - static_cast<int64_t>(previous_timestamp);
      if (solve_time < 1) solve_time = 1;
      if (solve_time > static_cast<int64_t>(6 * target_seconds)) {
        solve_time = 6 * target_seconds;
      }

      lwma_sum += solve_time * i;
      previous_timestamp = current_timestamp;
    }

    difficulty_type total_window_difficulty = 
        cumulative_difficulties.back() - cumulative_difficulties[count - n - 1];

    double denominator = static_cast<double>(n * (n + 1) / 2) * static_cast<double>(target_seconds);
    double avg_target = static_cast<double>(total_window_difficulty) / denominator;

    uint64_t raw_next_difficulty = static_cast<uint64_t>(avg_target * static_cast<double>(lwma_sum));

    // --- 100-BLOCK EPOCH TARGET ANCHOR ---
    // Evaluates actual time elapsed over the last 100 blocks vs expected time (100 * target_seconds).
    // Adjusts difficulty proportionally to anchor block production to target_seconds.
    if (count >= 101) {
      uint64_t actual_epoch_time = timestamps.back() - timestamps[count - 101];
      uint64_t expected_epoch_time = 100 * target_seconds;

      if (actual_epoch_time > 0) {
        double epoch_correction = static_cast<double>(expected_epoch_time) / static_cast<double>(actual_epoch_time);
        
        // Clamp epoch correction factor to [0.50, 2.00]
        epoch_correction = std::max(0.50, std::min(2.00, epoch_correction));
        raw_next_difficulty = static_cast<uint64_t>(static_cast<double>(raw_next_difficulty) * epoch_correction);
      }
    }

    // Asymmetric Step Clamp: Limit single-block difficulty shift to +/- 15% unless difficulty is low
    difficulty_type prev_diff = cumulative_difficulties.back() - cumulative_difficulties[count - 2];
    if (prev_diff > 10) {
      uint64_t max_diff = prev_diff * 115 / 100;
      uint64_t min_diff = prev_diff * 85 / 100;
      if (raw_next_difficulty > max_diff) raw_next_difficulty = max_diff;
      if (raw_next_difficulty < min_diff) raw_next_difficulty = min_diff;
    }

    return std::max(raw_next_difficulty, static_cast<uint64_t>(1));
  }

  bool check_pow_hash(const uint8_t pow_hash[32], difficulty_type target_difficulty) {
    uint64_t hash_val = 0;
    for (size_t i = 0; i < 8; ++i) {
      hash_val |= (static_cast<uint64_t>(pow_hash[i]) << (i * 8));
    }

    if (target_difficulty == 0) return false;
    uint64_t max_target = UINT64_MAX / target_difficulty;
    return hash_val <= max_target;
  }

} // namespace crypto
