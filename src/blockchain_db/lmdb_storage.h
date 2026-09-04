#pragma once

#include "../cryptonote_core/cryptonote_basic.h"
#include <string>
#include <vector>
#include <unordered_set>

namespace blockchain_db {

  /**
   * On-Disk Persistent Database Manager for RandomLite (XRL)
   */
  class PersistentDatabase {
  public:
    PersistentDatabase() = default;
    ~PersistentDatabase() = default;

    // Open/Create database directory & files
    bool open(const std::string &db_path);

    // Save block to disk
    bool save_block(const cryptonote::block &b, uint64_t height, uint64_t difficulty);

    // Load full chain state from disk
    bool load_chain(
        std::vector<cryptonote::block> &blocks,
        std::vector<uint64_t> &timestamps,
        std::vector<uint64_t> &cumulative_difficulties,
        std::unordered_set<std::string> &spent_key_images
    );

    // Save spent key image to disk
    bool save_key_image(const std::string &key_image_hex);

    // Close database
    void close();

  private:
    std::string m_db_path;
    bool m_opened = false;
  };

} // namespace blockchain_db
