#ifndef DEBUG_H
#define DEBUG_H

#include <bitset>
#include <iomanip>
#include <iostream>

class Cache;

/*
 * Convert integer to binary string of a certain number of bits.
 */
std::string i_to_binarystr(uint32_t x, size_t size);

/*
 * Debug print cache status.
 */
void debug_print(const Cache &c);

#endif
