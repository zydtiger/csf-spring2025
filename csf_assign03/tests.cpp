#include <dirent.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <map>
#include <regex>
#include <sstream>
#include <vector>

#include "args.h"
#include "cache.h"

#define ASSERT(condition)                                             \
  do {                                                                \
    if (!(condition)) {                                               \
      std::cerr << "Assertion failed: (" #condition "), function "    \
                << __FUNCTION__ << ", file " << __FILE__ << ", line " \
                << __LINE__ << ".\n";                                 \
      std::abort();                                                   \
    }                                                                 \
  } while (0)

std::string basename(std::string path) {
  int pos = path.find_last_of('/');
  std::string filename = path.substr(pos + 1);
  pos = filename.find('.');
  if (pos == -1) pos = filename.length();
  return filename.substr(0, pos);
}

std::vector<std::string> listdir(std::string dirpath) {
  std::vector<std::string> results;
  DIR* dir = opendir(dirpath.c_str());
  if (dir == nullptr) return results;  // return empty results
  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    // skip . and ..
    if (std::string(entry->d_name) == "." ||
        std::string(entry->d_name) == "..") {
      continue;
    }
    results.push_back(entry->d_name);
  }
  closedir(dir);
  return results;
}

std::string read_all(std::string path) {
  std::ifstream ifs(path);
  return {std::istreambuf_iterator<char>(ifs),
          std::istreambuf_iterator<char>()};
}

std::vector<std::string> split(const std::string& s, std::string delimiter) {
  std::regex rdelimiter(delimiter);
  return {std::sregex_token_iterator{s.begin(), s.end(), rdelimiter, -1},
          std::sregex_token_iterator{}};
}

std::string get_cmd_from_file(std::string filename) {
  std::vector<std::string> parts = split(basename(filename), "_");
  std::string cmd = "./csim";
  for (const std::string& part : parts) {
    if (part == "wa")
      cmd += " write-allocate";
    else if (part == "nwa")
      cmd += " no-write-allocate";
    else if (part == "wt")
      cmd += " write-through";
    else if (part == "wb")
      cmd += " write-back";
    else
      cmd += " " + part;
  }
  return cmd;
}

struct TestCase {
  std::string command;
  std::string result;
};

struct TestConfig {
  std::string trace_path;
  std::vector<TestCase> test_cases;
};

std::map<std::string, TestConfig> test_map;
void init_test() {
  std::string traces_path = "./tests/traces";
  std::string expected_path = "./tests/expected_results";

  std::vector<std::string> traces = listdir(traces_path);
  for (const std::string& trace_file : traces) {
    std::string full_path = traces_path + "/" + trace_file;
    std::string name = basename(trace_file);
    test_map[name] = {full_path, std::vector<TestCase>()};

    std::string expected_dir = expected_path + "/" + name;
    std::vector<std::string> expected_files = listdir(expected_dir);
    for (const std::string& expected_file : expected_files) {
      test_map[name].test_cases.push_back({
          get_cmd_from_file(expected_file),
          read_all(expected_dir + "/" + expected_file),
      });
    }
  }

  std::cout << std::string(25, '-') << " TEST CASES " << std::string(25, '-')
            << std::endl;
  std::cout << std::left << std::setw(12) << "Name";
  std::cout << std::left << std::setw(40) << "Trace Path";
  std::cout << std::left << std::setw(5) << "Test Count";
  std::cout << std::endl;
  for (auto pair : test_map) {
    std::cout << std::left << std::setw(12) << pair.first;
    std::cout << std::left << std::setw(40) << pair.second.trace_path;
    std::cout << std::left << std::setw(5) << pair.second.test_cases.size();
    std::cout << std::endl;
  }
  std::cout << std::string(25, '-') << " TEST CASES " << std::string(25, '-')
            << std::endl
            << std::endl;
}
void cleanup_test() { unlink("./output.txt"); }

void test_is_power_of_2();   // test power check
void test_args_correct();    // test args success
void test_args_incorrect();  // test args failed
void test_config(bool write_allocate, bool write_through, bool is_lru);

int main(void) {
  init_test();

  test_is_power_of_2();
  test_args_correct();
  test_args_incorrect();
  test_config(0, 1, 1);  // no-write-allocate + write-through + lru
  // test_config(0, 1, 0);  // no-write-allocate + write-through + fifo
  test_config(1, 1, 1);  // write-allocate + write-through + lru
  // test_config(1, 1, 0);  // write-allocate + write-through + fifo
  test_config(1, 0, 1);  // write-allocate + write-back + lru
  // test_config(1, 0, 0);  // write-allocate + write-back + fifo

  cleanup_test();

  return 0;
}

// test power check
void test_is_power_of_2() {
  ASSERT(is_power_of_2(1));
  ASSERT(is_power_of_2(4));
  ASSERT(is_power_of_2(128));
  ASSERT(is_power_of_2(256));

  ASSERT(!is_power_of_2(5));
  ASSERT(!is_power_of_2(127));
}

// test args success
void test_args_correct() {
  std::vector<std::string> argv1 = {
      "./csim", "1", "1", "4", "write-allocate", "write-through", "lru"};
  std::vector<std::string> argv2 = {
      "./csim", "1", "1", "4", "write-allocate", "write-through", "fifo"};
  std::vector<std::string> argv3 = {"./csim",         "1",          "1",  "4",
                                    "write-allocate", "write-back", "lru"};
  std::vector<std::string> argv4 = {"./csim",         "1",          "1",   "4",
                                    "write-allocate", "write-back", "fifo"};
  std::vector<std::string> argv5 = {
      "./csim", "1", "1", "4", "no-write-allocate", "write-through", "lru"};
  std::vector<std::string> argv6 = {
      "./csim", "1", "1", "4", "no-write-allocate", "write-through", "fifo"};

  CacheConfig config;
  ASSERT(parse_args(7, argv1, config));
  ASSERT(1 == config.num_sets);
  ASSERT(1 == config.num_blocks);
  ASSERT(4 == config.block_size);
  ASSERT(config.write_allocate);
  ASSERT(config.write_through);
  ASSERT(config.is_lru);

  ASSERT(parse_args(7, argv2, config));
  ASSERT(1 == config.num_sets);
  ASSERT(1 == config.num_blocks);
  ASSERT(4 == config.block_size);
  ASSERT(config.write_allocate);
  ASSERT(config.write_through);
  ASSERT(!config.is_lru);

  ASSERT(parse_args(7, argv3, config));
  ASSERT(1 == config.num_sets);
  ASSERT(1 == config.num_blocks);
  ASSERT(4 == config.block_size);
  ASSERT(config.write_allocate);
  ASSERT(!config.write_through);
  ASSERT(config.is_lru);

  ASSERT(parse_args(7, argv4, config));
  ASSERT(1 == config.num_sets);
  ASSERT(1 == config.num_blocks);
  ASSERT(4 == config.block_size);
  ASSERT(config.write_allocate);
  ASSERT(!config.write_through);
  ASSERT(!config.is_lru);

  ASSERT(parse_args(7, argv5, config));
  ASSERT(1 == config.num_sets);
  ASSERT(1 == config.num_blocks);
  ASSERT(4 == config.block_size);
  ASSERT(!config.write_allocate);
  ASSERT(config.write_through);
  ASSERT(config.is_lru);

  ASSERT(parse_args(7, argv6, config));
  ASSERT(1 == config.num_sets);
  ASSERT(1 == config.num_blocks);
  ASSERT(4 == config.block_size);
  ASSERT(!config.write_allocate);
  ASSERT(config.write_through);
  ASSERT(!config.is_lru);
}

// test args failed
void test_args_incorrect() {
  std::vector<std::string> argv1 = {
      "./csim", "1", "3", "4", "write-allocate", "write-through", "lru"};

  CacheConfig config;
  std::ostringstream oss;
  std::streambuf* original_cerr = std::cerr.rdbuf();
  std::cerr.rdbuf(oss.rdbuf());  // redirect output

  ASSERT(!parse_args(6, argv1, config));  // wrong argc
  ASSERT(
      "Usage: ./csim <sets> <blocks> <blocksize> "
      "<write-allocate|no-write-allocate> <write-through|write-back> "
      "<lru|fifo>\n" == oss.str());
  oss.str("");

  ASSERT(!parse_args(7, argv1, config));  // wrong num_blocks
  ASSERT("Error: Sets, blocks, and block size must be powers of 2\n" ==
         oss.str());
  std::cerr.rdbuf(original_cerr);  // reset
}

void test_config(bool write_allocate, bool write_through, bool is_lru) {
  std::cout << std::string(25, '-')
            << std::string(" test_") + (write_allocate ? "wa_" : "nwa_") +
                   (write_through ? "wt_" : "wb_") + (is_lru ? "lru" : "fifo")
            << std::string(25, '-') << std::endl;

  bool passed = true;
  for (auto [name, test_config] : test_map) {
    for (auto test_case : test_config.test_cases) {
      CacheConfig config;
      parse_args(7, split(test_case.command, " "), config);
      if (config.write_allocate == write_allocate &&
          config.write_through == write_through && config.is_lru == is_lru) {
        std::string command = "cat " + test_config.trace_path + " | " +
                              test_case.command + " > ./output.txt" + " 2>&1";
        system(command.c_str());  // run command

        std::cout << std::left << std::setw(100)
                  << test_case.command + " < " + test_config.trace_path;
        if (test_case.result != read_all("./output.txt")) {
          std::cout << "..... failed!!!!!" << std::endl;
          std::cout << "Reference:" << std::endl
                    << test_case.result << std::endl;
          std::cout << "Actual:" << std::endl
                    << read_all("./output.txt") << std::endl;
          passed = false;
        } else
          std::cout << "..... passed" << std::endl;
      }
    }
  }

  std::cout << std::string(25, '-') << (passed ? " PASS! " : " FAIL! ")
            << std::string(25, '-') << std::endl
            << std::endl;
}
