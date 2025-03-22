#include "cache.h"

void test_args_correct();    // test args success
void test_args_incorrect();  // test args failed
void test_nwa_wt_lru();      // no-write-allocate + write-through + lru
void test_nwa_wt_fifo();     // no-write-allocate + write-through + fifo
void test_nwa_wb_lru();      // no-write-allocate + write-back + lru
void test_nwa_wb_fifo();     // no-write-allocate + write-back + fifo
void test_wa_wt_lru();       // write-allocate + write-through + lru
void test_wa_wt_fifo();      // write-allocate + write-through + fifo
void test_wa_wb_lru();       // write-allocate + write-back + lru
void test_wa_wb_fifo();      // write-allocate + write-back + fifo

int main(void) {
  test_args_correct();
  test_args_incorrect();
  test_nwa_wt_lru();
  test_nwa_wt_fifo();
  test_nwa_wb_lru();
  test_nwa_wb_fifo();
  test_wa_wt_lru();
  test_wa_wt_fifo();
  test_wa_wb_lru();
  test_wa_wb_fifo();

  return 0;
}

// test args success
void test_args_correct() {}

// test args failed
void test_args_incorrect() {}

// no-write-allocate + write-through + lru
void test_nwa_wt_lru() {}

// no-write-allocate + write-through + fifo
void test_nwa_wt_fifo() {}

// no-write-allocate + write-back + lru
void test_nwa_wb_lru() {}

// no-write-allocate + write-back + fifo
void test_nwa_wb_fifo() {}

// write-allocate + write-through + lru
void test_wa_wt_lru() {}

// write-allocate + write-through + fifo
void test_wa_wt_fifo() {}

// write-allocate + write-back + lru
void test_wa_wb_lru() {}

// write-allocate + write-back + fifo
void test_wa_wb_fifo() {}