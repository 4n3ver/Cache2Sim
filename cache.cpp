//
// Created by lennart on 1/27/16.
//

#include "cache.h"

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

        uint64_t set_size = ((uint64_t) 1) << s;

        data = new block_ptr[lim_index];
        for (uint64_t i = 0; i < lim_index; i++) {
            data[i] = new block[set_size];
            data[i] = {0};
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
        DEBUG_PRINT("Set size: %lu Block/line\n", set_size);

        DEBUG_PRINT("Offset mask: %lx\n", mask_off);
        DEBUG_PRINT("Index mask: %lx\n", mask_index);
        DEBUG_PRINT("Tag mask: %lx\n", mask_tag);

        DEBUG_PRINT("Index shift: %d\n", shift_index);
        DEBUG_PRINT("Tag shift: %d\n", shift_tag);
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
