#include "debug.h"

#include <cmath>

#include "cache.h"

std::string i_to_binarystr(uint32_t x, size_t size) {
  std::string bit_str = std::bitset<32>(x).to_string();
  bit_str = bit_str.substr(bit_str.length() - size);
  for (size_t i = 4; i < bit_str.length(); i += 5) bit_str.insert(i, " ");
  return bit_str;
}

void debug_print(const Cache &c) {
  std::cout << std::string(80, '-') << std::endl;
  std::cout << "Index ";
  std::cout << std::string(19, ' ') << "Tag" << std::string(19, ' ');
  std::cout << "Valid ";
  std::cout << "Dirty " << std::endl;

  size_t tag_bits = 32 - log2(c.config.num_sets) - log2(c.config.block_size);

  for (uint32_t i = 0; i < c.config.num_sets; i++) {
    bool is_set_any_valid = false;
    for (uint32_t j = 0; j < c.config.num_blocks; j++)
      if (c.sets[i][j].valid) {
        is_set_any_valid = true;
        break;
      }
    if (!is_set_any_valid) continue;

    std::cout << std::left << std::setw(5) << i;

    for (uint32_t j = 0; j < c.config.num_blocks; j++) {
      std::string tag_str = i_to_binarystr(c.sets[i][j].tag, tag_bits);

      if (j != 0) std::cout << std::string(5, ' ');
      std::cout << " " << std::right << std::setw(40) << tag_str;
      std::cout << " " << std::right << std::setw(5) << c.sets[i][j].valid;
      std::cout << " " << std::right << std::setw(5) << c.sets[i][j].dirty;
      std::cout << std::endl;
    }
  }

  std::cout << std::string(80, '-') << std::endl << std::endl;
}