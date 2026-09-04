#include "lmdb_storage.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace blockchain_db {

  bool PersistentDatabase::open(const std::string &db_path) {
    m_db_path = db_path;
    m_opened = true;
    return true;
  }

  bool PersistentDatabase::save_block(const cryptonote::block &b, uint64_t height, uint64_t difficulty) {
    if (!m_opened) return false;

    std::string blocks_file = m_db_path + "/blocks.dat";
    std::ofstream out(blocks_file, std::ios::binary | std::ios::app);
    if (!out.is_open()) return false;

    out.write(reinterpret_cast<const char*>(&height), sizeof(height));
    out.write(reinterpret_cast<const char*>(&b.timestamp), sizeof(b.timestamp));
    out.write(reinterpret_cast<const char*>(&b.nonce), sizeof(b.nonce));
    out.write(reinterpret_cast<const char*>(&difficulty), sizeof(difficulty));
    out.write(reinterpret_cast<const char*>(b.hash.data()), 32);

    return true;
  }

  bool PersistentDatabase::save_key_image(const std::string &key_image_hex) {
    if (!m_opened) return false;

    std::string keys_file = m_db_path + "/key_images.dat";
    std::ofstream out(keys_file, std::ios::app);
    if (!out.is_open()) return false;

    out << key_image_hex << "\n";
    return true;
  }

  bool PersistentDatabase::load_chain(
      std::vector<cryptonote::block> &blocks,
      std::vector<uint64_t> &timestamps,
      std::vector<uint64_t> &cumulative_difficulties,
      std::unordered_set<std::string> &spent_key_images
  ) {
    if (!m_opened) return false;

    std::string blocks_file = m_db_path + "/blocks.dat";
    std::ifstream in(blocks_file, std::ios::binary);
    if (!in.is_open()) return false;

    uint64_t cum_diff = 0;
    while (in.peek() != EOF) {
      uint64_t height = 0, timestamp = 0, difficulty = 0;
      uint32_t nonce = 0;
      cryptonote::crypto_hash hash_val;

      in.read(reinterpret_cast<char*>(&height), sizeof(height));
      in.read(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));
      in.read(reinterpret_cast<char*>(&nonce), sizeof(nonce));
      in.read(reinterpret_cast<char*>(&difficulty), sizeof(difficulty));
      in.read(reinterpret_cast<char*>(hash_val.data()), 32);

      if (in.gcount() < 32) break;

      cryptonote::block b{};
      b.timestamp = timestamp;
      b.nonce = nonce;
      b.hash = hash_val;

      blocks.push_back(b);
      timestamps.push_back(timestamp);
      cum_diff += difficulty;
      cumulative_difficulties.push_back(cum_diff);
    }

    // Load key images
    std::string keys_file = m_db_path + "/key_images.dat";
    std::ifstream in_keys(keys_file);
    if (in_keys.is_open()) {
      std::string line;
      while (std::getline(in_keys, line)) {
        if (!line.empty()) spent_key_images.insert(line);
      }
    }

    std::cout << "[LMDB Persistence] Loaded " << blocks.size() << " blocks & " 
              << spent_key_images.size() << " spent key images from disk." << std::endl;
    return true;
  }

  void PersistentDatabase::close() {
    m_opened = false;
  }

} // namespace blockchain_db
