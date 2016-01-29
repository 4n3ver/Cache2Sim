//
// Created by lennart on 1/27/16.
//

#ifndef CACHE2SIM_CACHE_H
#define CACHE2SIM_CACHE_H

#define DEBUG_CACHE
#ifdef DEBUG_CACHE

    #include <stdio.h>

    #define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
    #define DEBUG_IF(x) x
#else
    #define DEBUG_PRINT(...)
    #define DEBUG_IF(x)
#endif

#include <string>
#include <errno.h>
#include <stdint.h>

typedef struct s_block {
    uint64_t tag;
    bool dirty;
    bool valid;
} block, *block_ptr;

class cache {
    private:
        uint8_t shift_index;
        uint8_t shift_tag;

        uint64_t lim_off;
        uint64_t lim_index;
        uint64_t lim_tag;

        uint64_t mask_off;
        uint64_t mask_index;
        uint64_t mask_tag;

        block_ptr *data;

    public:
        cache(int c, int b, int s);

        uint64_t index_of(uint64_t address) {
            return (address >> shift_index) & mask_index;
        }

        uint64_t tag_of(uint64_t address) {
            return (address >> shift_tag) & mask_tag;
        }

        uint64_t offset_of(uint64_t address) {
            return address & mask_off;
        }

        ~cache();
};
#endif //CACHE2SIM_CACHE_H
