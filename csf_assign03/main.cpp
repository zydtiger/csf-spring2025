#include <iostream>

#include "args.h"
#include "cache.h"

int main(int argc, char **argv) {
  // parse args
  CacheConfig config;
  std::vector<std::string> args(argc);
  for (int i = 0; i < argc; i++) args.push_back(argv[i]);
  if (!parse_args(argc, args, config)) return 1;  // error termination

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
