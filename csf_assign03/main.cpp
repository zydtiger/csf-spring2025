#include <iostream>
#include <unordered_map>
#include <vector>

// Cache configuration from command args
struct CacheConfig {
  int num_sets;
  int num_blocks;
  int block_size;
  bool write_allocate;  // 0 is no-write-allocate, 1 is write-allocate
  bool write_through;   // 0 is write-back, 1 is write-through
  bool is_lru;          // 0 is FIFO, 1 is LRU
};

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

  int load_count = 0;
  int store_count = 0;

  std::string line;
  while (std::getline(std::cin, line)) {
    if (line[0] == 'l') {
      load_count++;
    } else {
      store_count++;
    }
  }

  printf("Total loads: %d\n", load_count);
  printf("Total stores: %d\n", store_count);
  printf("Load hits: %d\n", -1);
  printf("Load misses: %d\n", -1);
  printf("Store hits: %d\n", -1);
  printf("Store misses: %d\n", -1);
  printf("Total cycles: %d\n", -1);

  return 0;
}
