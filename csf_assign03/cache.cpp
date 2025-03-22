#include "cache.h"

int Set::find_hit(uint32_t tag) {
  int slot_index = -1;
  for (int i = 0; i < slots.size(); i++) {
    if (slots[i].tag == tag) {  // check for hit
      slot_index = i;
      break;
    }
  }
  return slot_index;
}

int Set::find_victim_slot() {
  int slot_index = -1;
  for (int i = 0; i < slots.size(); i++) {
    Slot &slot = slots[i];
    if (!slot.valid ||  // found empty slot or least used slot
        slot.access_order == slots.size() - 1) {
      slot_index = i;
      break;
    }
  }
  return slot_index;
}

void Set::update_lru(int reference) {
  for (Slot &slot : slots) {
    if (slot.valid && slot.access_order < reference) {
      slot.access_order++;
    }
  }
}

Cache::Cache(CacheConfig config) : config(config) {
  this->sets.resize(config.num_sets, Set(config.num_blocks));
  this->stats = {.total_loads = 0,
                 .total_stores = 0,
                 .load_hits = 0,
                 .load_misses = 0,
                 .store_hits = 0,
                 .store_misses = 0,
                 .total_cycles = 0};
}

void Cache::load(uint32_t address) {
  this->stats.total_loads++;
  // find index for set, tag
  uint32_t tag = get_tag(address);
  size_t index = get_index(address);

  Set &set = this->sets[index];
  int slot_index = set.find_hit(tag);

  if (slot_index != -1) {  // hit!!!!
    this->stats.load_hits++;
    this->stats.total_cycles++;  // hit takes 1 cycle

    // increment access order that is smaller than the current hit
    set.update_lru(set[slot_index].access_order);
    set[slot_index].access_order = 0;

  } else {  // if miss, load in memory and set valid = 1, increase all counters
    this->stats.load_misses++;

    // find a slot to overwrite
    slot_index = set.find_victim_slot();
    if (set[slot_index].valid && set[slot_index].dirty &&
        !config.write_through) {
      // memory write of current cache takes 100 cycles
      this->stats.total_cycles += 100;
    }
    set.update_lru(config.num_blocks);  // increment all slots
    set[slot_index] = {
        .tag = tag, .valid = 1, .dirty = false, .access_order = 0};

    // memory access takes 100 cycles per 4 bytes
    this->stats.total_cycles += 1 + 100 * config.block_size / 4;
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

    // hit takes 1 cycles, ram takes 100 cycles
    if (config.write_through) {
      this->stats.total_cycles += 101;
    } else {
      // write to cache only
      set[slot_index].dirty = true;
      this->stats.total_cycles++;  // cache takes 1 cycle
    }

    // increment access order that is smaller than the current hit
    set.update_lru(set[slot_index].access_order);
    set[slot_index].access_order = 0;

  } else {
    this->stats.store_misses++;

    if (!config.write_allocate) {
      this->stats.total_cycles += 100;  // write directly to ram
      return;
    }

    // find a slot to overwrite
    slot_index = set.find_victim_slot();
    if (set[slot_index].valid && set[slot_index].dirty &&
        !config.write_through) {
      // memory write of current cache takes 100 cycles
      this->stats.total_cycles += 100;
    }

    set.update_lru(config.num_blocks);  // increment all slots
    set[slot_index] = {
        .tag = tag, .valid = 1, .dirty = false, .access_order = 0};

    // load into cache, memory access takes 100 cycles per 4 bytes
    this->stats.total_cycles += 100 * config.block_size / 4;

    if (config.write_through) {
      set[slot_index].dirty = false;
      this->stats.total_cycles += 101;
    } else {
      set[slot_index].dirty = true;
      this->stats.total_cycles += 1;
    }
  }

  //!
  // debug_print(*this);
}