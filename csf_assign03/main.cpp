#include <iostream>

#include "cache.h"

/*
 * Check if a number is a power of 2.
 */
bool is_power_of_2(int n) { return n > 0 && (n & (n - 1)) == 0; }

/*
 * Parse command line arguments and validate them.
 */
bool parseArgs(int argc, char *argv[], CacheConfig &config) {
  if (argc != 7) {
    std::cerr
        << "Usage: " << argv[0]
        << " <sets> <blocks> <blocksize> <write-allocate|no-write-allocate> "
           "<write-through|write-back> <lru|fifo>"
        << std::endl;
    return false;
  }

  config.num_sets = std::atoi(argv[1]);
  config.num_blocks = std::atoi(argv[2]);
  config.block_size = std::atoi(argv[3]);

  // check if parameters are valid
  if (!is_power_of_2(config.num_sets) || !is_power_of_2(config.num_blocks) ||
      !is_power_of_2(config.block_size)) {
    std::cerr << "Error: Sets, blocks, and block size must be powers of 2"
              << std::endl;
    return false;
  }
  if (config.block_size < 4) {
    std::cerr << "Error: Block size must be at least 4 bytes" << std::endl;
    return false;
  }

  // parse write-allocate option
  std::string write_allocate_str = argv[4];
  if (write_allocate_str == "write-allocate") {
    config.write_allocate = true;
  } else if (write_allocate_str == "no-write-allocate") {
    config.write_allocate = false;
  } else {
    std::cerr << "Error: Invalid write-allocate option" << std::endl;
    return false;
  }

  // parse write-through option
  std::string writeThroughStr = argv[5];
  if (writeThroughStr == "write-through") {
    config.write_through = true;
  } else if (writeThroughStr == "write-back") {
    config.write_through = false;
  } else {
    std::cerr << "Error: Invalid write-through option" << std::endl;
    return false;
  }

  // check for invalid combination
  if (!config.write_allocate && !config.write_through) {
    std::cerr << "Error: Cannot combine no-write-allocate with write-back"
              << std::endl;
    return false;
  }

  // parse LRU/FIFO arg
  std::string replacementStr = argv[6];
  if (replacementStr == "lru") {
    config.is_lru = true;
  } else if (replacementStr == "fifo") {
    config.is_lru = false;
  } else {
    std::cerr << "Error: Invalid replacement policy" << std::endl;
    return false;
  }

  return true;
}

int main(int argc, char **argv) {
  // parse args
  CacheConfig config;
  if (!parseArgs(argc, argv, config)) return 1;  // error termination

  Cache cache(config);

  std::string line;
  while (std::getline(std::cin, line)) {
    std::string address_str = line.substr(2, 10);
    uint32_t address = std::stoul(address_str, nullptr, 16);
    if (line[0] == 'l') {
      cache.load(address);
    } else {
      cache.save(address);
    }
  }

  CacheStats stats = cache.get_stats();

  std::cout << "Total loads: " << stats.total_loads << std::endl;
  std::cout << "Total stores: " << stats.total_stores << std::endl;
  std::cout << "Load hits: " << stats.load_hits << std::endl;
  std::cout << "Load misses: " << stats.load_misses << std::endl;
  std::cout << "Store hits: " << stats.store_hits << std::endl;
  std::cout << "Store misses: " << stats.store_misses << std::endl;
  std::cout << "Total cycles: " << stats.total_cycles << std::endl;

  return 0;
}
