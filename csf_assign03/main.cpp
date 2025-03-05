#include <iostream>

int main( int argc, char **argv ) {
  if (argc != 7) return -1;
  
  int load_count = 0;
  int store_count = 0;

  std::string line;
  while (std::getline(std::cin, line)) {
    if (line[0] == 'l') load_count++;
    else store_count++;
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
