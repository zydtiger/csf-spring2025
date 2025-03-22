#ifndef DEBUG_H
#define DEBUG_H

#include <bitset>
#include <iomanip>
#include <iostream>

class Cache;

std::string i_to_binarystr(uint32_t x, size_t size);
void debug_print(const Cache &c);

#endif
