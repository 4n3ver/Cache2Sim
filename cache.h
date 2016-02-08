/**
 * Header file of the not-so-object-oriented cache object
 *
 * @author  Yoel Ivan (yivan3@gatech.edu)
 * @version 0.0a
 */

#ifndef CACHE2SIM_CACHE_H
#define CACHE2SIM_CACHE_H

#define DEBUG_CACHE
#ifdef DEBUG_CACHE

    #include <stdio.h>
    #include <assert.h>

    #define DEBUG_PRINT(...) /*fprintf(stderr, __VA_ARGS__)*/
    #define DEBUG_IF(x) x
    #define DEBUG_ASSERT(x) assert(x)
#else
    #define DEBUG_PRINT(...)
    #define DEBUG_IF(x) false
    #define DEBUG_ASSERT(x)
#endif

#include <string>
#include <errno.h>
#include <stdint.h>
#include <functional>

/* Simple block struct that are not supposed to be exposed to user, but... */
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

/* Cache object, consists of 2-d array of block, row is index, col is set */
class cache {

    /* This is private. */
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

        int s;

        block_ptr fetch_block(uint64_t address,
                              std::function<bool (block_ptr)> cond);

        uint64_t index_of(uint64_t address) {
            return (address >> shift_index) & mask_index;
        }

        uint64_t tag_of(uint64_t address) {
            return (address >> shift_tag) & mask_tag;
        }

        uint64_t offset_of(uint64_t address) {
            return address & mask_off;
        }

    public:
        /* Error code so user have something to throw in case of exception */
        static const int ACCESS_MISS = 99;
        static const int SET_FULL = 1099;

        /**
         * Constructor of this object
         *
         * @c   log base 2 of the size of the cache in byte
         * @b   log base 2 of the size of each block in byte
         * @s   log base 2 of the size of each set in block
         */
        cache(int c, int b, int s);

        /**
         * Return this cache hit time, based on log base 2 of the size of each set in block
         */
        int get_s() {
            return s;
        }

        /**
         * Gives mask to hide the offset based on this cache block size.
         *
         * @return  mask to hide the offset (zero out the offset by using bitwise &).
         */
        uint64_t getMask_off() const {
            return mask_off;
        }

        /**
         * Access this cache.
         *
         * @address address of the block to look for
         * @throw   ACCESS_MISS if block not in this cache
         * @return  block where @address passed reside.
         */
        block_ptr access(uint64_t address);

        /**
         * Find empty block where @address can be placed.
         *
         * @address address of the block to look for
         * @return  empty block where @address can be placed
         */
        block_ptr find_empty_block(uint64_t address);

        /**
         * Store @address at @location
         *
         * @address     address to store
         * @location    location in the cache where @address will be stored
         */
        void store_block(uint64_t address, block_ptr location);

        /**
         * Find LRU block based on @address.
         *
         * @address address that need to be placed in the LRU block, once the LRU block evicted
         * @return  LRU block to be evicted
         */
        block_ptr find_victim_block(uint64_t address);

        /* Plain old destructor */
        ~cache();
};
#endif //CACHE2SIM_CACHE_H
