#ifndef CACHE_H
#define CACHE_H

#include <vector>

#include "debug.h"

// Actual slot that mocks cache
struct Slot {
  uint32_t tag;
  bool valid;
  bool dirty;
  uint32_t access_order;  // 0 = frequently used
};

// Each set of slots
class Set {
 private:
  std::vector<Slot> slots;

 public:
  Set(size_t size) : slots(size) {}
  int find_hit(uint32_t tag);
  int find_victim_slot();
  void update_lru(int reference);
  Slot& operator[](size_t index) { return this->slots[index]; }
  const Slot& operator[](size_t index) const { return this->slots[index]; }
};

// Cache configuration from command args
struct CacheConfig {
  uint32_t num_sets;
  uint32_t num_blocks;
  uint32_t block_size;
  bool write_allocate;  // 0 is no-write-allocate, 1 is write-allocate
  bool write_through;   // 0 is write-back, 1 is write-through
  bool is_lru;          // 0 is FIFO, 1 is LRU
};

// Cache stats struct
struct CacheStats {
  uint32_t total_loads;
  uint32_t total_stores;
  uint32_t load_hits;
  uint32_t load_misses;
  uint32_t store_hits;
  uint32_t store_misses;
  uint64_t total_cycles;
};

// Cache class
class Cache {
 private:
  std::vector<Set> sets;
  CacheStats stats;
  CacheConfig config;  // store config

  // helper functions
  uint32_t get_tag(uint32_t address) const {
    return address / (config.num_sets * config.block_size);
  }
  size_t get_index(uint32_t address) const {
    return (address / config.block_size) % config.num_sets;
  }

 public:
  Cache(CacheConfig config);    // prepare cache as defined in the config
  void load(uint32_t address);  // load address 'l'
  void save(uint32_t address);  // save address 's'
  CacheStats get_stats() const { return this->stats; }

  friend void debug_print(const Cache& c);
};

#endif
