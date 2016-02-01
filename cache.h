//
// Created by lennart on 1/27/16.
//

#ifndef CACHE2SIM_CACHE_H
#define CACHE2SIM_CACHE_H

#define DEBUG_CACHE
#ifdef DEBUG_CACHE

    #include <stdio.h>
    #include <assert.h>

    #define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
    #define DEBUG_IF(x) x
    #define DEBUG_ASSERT(x) assert(x)
#else
    #define DEBUG_PRINT(...)
    #define DEBUG_IF(x)
    #define DEBUG_ASSERT(x)
#endif

#include <string>
#include <errno.h>
#include <stdint.h>
#include <functional>

typedef struct s_block {
    uint64_t tag;
    uint64_t last_access_time;
    uint64_t address;
    bool dirty;
    bool valid;

    s_block() {
        reset();
    }

    void reset() {
        tag = 0;
        last_access_time = 0;
        address = 0;
        dirty = false;
        valid = false;
    }
} block, *block_ptr;

class cache {
    private:
        uint64_t clock;

        uint8_t shift_index;
        uint8_t shift_tag;

        uint64_t lim_off;
        uint64_t lim_index;
        uint64_t lim_tag;
        uint64_t lim_set;

        uint64_t mask_off;
        uint64_t mask_index;
        uint64_t mask_tag;

        block_ptr *data;

        block_ptr fetch_block(uint64_t address,
                              std::function<bool (block_ptr)> cond);

    public:
        static const int ACCESS_MISS = 99;
        static const int SET_FULL = 1099;

        cache(int c, int b, int s);

        uint64_t getMask_off() const {
            return mask_off;
        }

        // TODO: make this private once tested
        uint64_t index_of(uint64_t address) {
            return (address >> shift_index) & mask_index;
        }

        // TODO: make this private once tested
        uint64_t tag_of(uint64_t address) {
            return (address >> shift_tag) & mask_tag;
        }

        // TODO: make this private once tested
        uint64_t offset_of(uint64_t address) {
            return address & mask_off;
        }

        block_ptr access(uint64_t address);

        block_ptr find_empty_block(uint64_t address);

        void store_block(uint64_t address, block_ptr location);

        block_ptr find_victim_block(uint64_t address);

        ~cache();
};
#endif //CACHE2SIM_CACHE_H
