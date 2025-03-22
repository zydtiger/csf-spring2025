#ifndef ARGS_H
#define ARGS_H

#include "cache.h"

/*
 * Check if a number is a power of 2.
 */
bool is_power_of_2(int n);

/*
 * Parse command line arguments and validate them.
 */
bool parse_args(int argc, std::vector<std::string> argv, CacheConfig &config);

#endif
