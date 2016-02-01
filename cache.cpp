//
// Created by lennart on 1/27/16.
//

#include <string.h>
#include "cache.h"


block_ptr cache::fetch_block(uint64_t address,
                             std::function<bool (block_ptr)> cond) {
    uint64_t index = index_of(address);

    block_ptr set = data[index];
    for (int i = 0; i < lim_set; i++) {
        if (cond(&set[i])) {
            return &set[i];
        }
    }
    return nullptr;
}

cache::cache(int c, int b, int s) {
    /* TODO: add more condition to ensure valid args */
    if (c < 64 && c >= 0 && b + s <= c && b >= 0 && s >= 0) {
        shift_index = (uint8_t) b;
        shift_tag = (uint8_t) (c - s);

        lim_off = ((uint64_t) 1) << b;
        lim_index = ((uint64_t) 1) << (c - (b + s));
        lim_tag = ((uint64_t) 1) << (64 - (c - s));

        mask_off = lim_off - 1;
        mask_index = lim_index - 1;
        mask_tag = lim_tag - 1;

        lim_set = ((uint64_t) 1) << s;

        data = new block_ptr[lim_index];
        for (uint64_t i = 0; i < lim_index; i++) {
            data[i] = new block[lim_set];
        }

        if (DEBUG_IF(s == 0)) {
            DEBUG_PRINT("Direct-mapped cache\n");
        }
        else if (DEBUG_IF(s == c - b)) {
            DEBUG_PRINT("Fully-associative cache\n");
        }
        else {
            DEBUG_PRINT("%d-way associative cache\n", 1 << s);
        }

        DEBUG_PRINT("C: %d B: %d S: %d\n", c, b, s);

        DEBUG_PRINT("Cache size: %lu Byte\n", ((uint64_t) 1) << c);
        DEBUG_PRINT("Block size: %lu Byte\n", ((uint64_t) 1) << b);
        DEBUG_PRINT("Set size: %lu Block/line\n", lim_set);

        DEBUG_PRINT("Offset mask: %lx\n", mask_off);
        DEBUG_PRINT("Index mask: %lx\n", mask_index);
        DEBUG_PRINT("Tag mask: %lx\n", mask_tag);

        DEBUG_PRINT("Index shift: %d\n", shift_index);
        DEBUG_PRINT("Tag shift: %d\n", shift_tag);

        for (int index = 0; index < lim_index; index++) {
            for (int set = 0; set < lim_set; set++) {
                DEBUG_ASSERT(data[index][set].tag == 0);
                DEBUG_ASSERT(data[index][set].address == 0);
                DEBUG_ASSERT(data[index][set].last_access_time == 0);
                DEBUG_ASSERT(!data[index][set].dirty);
                DEBUG_ASSERT(!data[index][set].valid);
            }
        }
        DEBUG_PRINT("\nInit test passed...\n");
        clock = 0;
    } else {
        throw EINVAL;
    }

}

cache::~cache() {
    for (uint64_t i = 0; i < lim_index; i++) {
        delete data[i];
    }
    delete data;
}

// row == index
// col == set
block_ptr cache::access(uint64_t address) {
    clock++;
    uint64_t tag = tag_of(address);
    block_ptr ret = fetch_block(address, [tag](block_ptr blck) -> bool {
        return blck->valid && blck->tag == tag;
    });

    if (ret != nullptr) {
        ret->last_access_time = clock;
        return ret;
    } else {
        throw ACCESS_MISS;
    }
}

void cache::store_block(uint64_t address, block_ptr location) {
    location->tag = tag_of(address);
    location->address = address & ~mask_off;
    location->last_access_time = clock;
    location->valid = true;
}

block_ptr cache::find_victim_block(uint64_t address) {
    uint64_t index = index_of(address);

    block_ptr set = data[index];
    block_ptr lru = &set[0];
    uint64_t min = lru->last_access_time;
    for (int i = 1; i < lim_set; i++) {
        if (min > set[i].last_access_time) {
            lru = &set[i];
            min = lru->last_access_time;
        }
    }
    return lru;
}

block_ptr cache::find_empty_block(uint64_t address) {
    block_ptr ret = fetch_block(address, [](block_ptr blck) -> bool {
        return !(blck->valid);
    });

    if (ret != nullptr) {
        return ret;
    } else {
        throw SET_FULL;
    }
}
