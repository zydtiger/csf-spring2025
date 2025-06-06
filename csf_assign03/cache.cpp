#include "cache.h"

int Set::find_hit(uint32_t tag) {
  for (size_t i = 0; i < valid_count; i++)
    if (slots[i].tag == tag) return static_cast<int>(i);
  return -1;
}

int Set::find_victim_slot(bool is_lru) {
  if (valid_count == slots.size()) {
    if(is_lru){
      int lru_index = 0;
      for (size_t i = 0; i < valid_count; i++) {
        if (slots[i].access_order > slots[lru_index].access_order) {
          lru_index = i;
        }
      }
      return lru_index;
    } else {
      int fifo_index = 0;
      for (size_t i = 0; i < valid_count; i++) {
        if (slots[i].access_order < slots[fifo_index].access_order) {
          fifo_index = i;
        }
      }
      return fifo_index;
    }
  }
  return valid_count++;
}

void Set::update_lru(uint32_t reference) {
  for (size_t i = 0; i < valid_count; i++) {
    if (slots[i].access_order < reference) {
      slots[i].access_order++;
    }
  }
}

Cache::Cache(CacheConfig config) : config(config) {
  this->sets.resize(config.num_sets, Set(config.num_blocks));
  this->stats = {0, 0, 0, 0, 0, 0, 0};
}

void Cache::load(uint32_t address) {
  this->stats.total_loads++;
  // find index for set, tag
  uint32_t tag = get_tag(address);
  size_t index = get_index(address);

  Set &set = sets[index];
  int slot_index = set.find_hit(tag);

  if (slot_index != -1) {  // hit!!!!
    stats.load_hits++;
    stats.total_cycles++;  // hit takes 1 cycle
    if (config.is_lru){
      // increment access order that is smaller than the current hit
      uint32_t old_order = set[slot_index].access_order;
      set.update_lru(old_order);
      set[slot_index].access_order = 0;
    }

  } else {  // if miss, load in memory and set valid = 1, increase all counters
    this->stats.load_misses++;

    // find a slot to overwrite
    slot_index = set.find_victim_slot(config.is_lru);
    if (set[slot_index].dirty && !config.write_through) {
      // dump occupied, memory access takes 100 cycles per 4 bytes
      this->stats.total_cycles += 100ULL * config.block_size / 4;
    }
    if (config.is_lru){
      set.update_lru(config.num_blocks);  // increment all slots
      set[slot_index] = {tag, false, 0};
    } else {
      set.mark_insertion_fifo(slot_index);
      set[slot_index].tag = tag;
      set[slot_index].dirty = false;
    }


    // load from ram and then cache
    this->stats.total_cycles += 1 + 100ULL * config.block_size / 4;
  }

  //!
  // debug_print(*this);
}

void Cache::save(uint32_t address) {
  this->stats.total_stores++;
  // find index for set, tag
  uint32_t tag = get_tag(address);
  size_t index = get_index(address);

  Set &set = this->sets[index];
  int slot_index = set.find_hit(tag);

  if (slot_index != -1) {  // hit!!!!
    this->stats.store_hits++;

    if (config.write_through) {
      this->stats.total_cycles += 100;  // write to memory directly
      if (config.write_allocate)
        this->stats.total_cycles++;  // also write to cache to sync
    } else {
      set[slot_index].dirty = true;
      this->stats.total_cycles++;  // write to cache only
    }

    // increment access order that is smaller than the current hit
    if (config.is_lru){
      set.update_lru(set[slot_index].access_order);
      set[slot_index].access_order = 0;
    }

  } else {
    stats.store_misses++;

    if (!config.write_allocate) {
      // write to ram directly
      this->stats.total_cycles += 100;
      return;
    }

    // find a slot to overwrite
    slot_index = set.find_victim_slot(config.is_lru);
    if (set[slot_index].dirty && !config.write_through) {
      // write occupied to ram
      this->stats.total_cycles += 100 * config.block_size / 4;
    }
    if (config.is_lru){
      set.update_lru(config.num_blocks);  // increment all slots
      set[slot_index] = {tag, false, 0};
    } else {
      set.mark_insertion_fifo(slot_index);
      set[slot_index].tag = tag;
      set[slot_index].dirty = false; 
    }
    // load from ram
    set[slot_index].tag = tag;
    set[slot_index].dirty = false;
    this->stats.total_cycles += 100 * config.block_size / 4;

    if (config.write_through) {
      set[slot_index].dirty = false;
      this->stats.total_cycles += 101;  // write to cache & ram
    } else {
      set[slot_index].dirty = true;
      this->stats.total_cycles += 1;  // write to cache only
    }
  }

  //!
  // debug_print(*this);
}