#include "cachesim.hpp"
#include "cache.h"
#include <deque>
#include <stdlib.h>

static cache *l1;
static cache *l2;
static uint64_t MAX_VC_SIZE;
static std::deque<uint64_t> vc;

static const int VC_MISS = 979;

static void swap_victim(uint64_t target, block_ptr victim) {
    DEBUG_ASSERT(!(victim->dirty));
    target = target & ~(l1->getMask_off());
    for (int i = 0; i < vc.size(); i++) {
        if (vc[i] == target) {
            // swapping and return
            uint64_t temp = vc[i];
            vc[i] = victim->address;
            victim->address = temp;
            DEBUG_ASSERT(victim->address & l1->getMask_off() == 0);
            return;
        }
    }
    // miss @ VC
    DEBUG_ASSERT(vc.size() <= MAX_VC_SIZE);
    if (vc.size() == MAX_VC_SIZE) {
        vc.pop_front();
    }
    DEBUG_ASSERT(victim->address & l1->getMask_off() == 0);
    vc.push_back(victim->address);
    throw VC_MISS;
}

/**
 * Subroutine for initializing the cache. You many add and initialize any global or heap
 * variables as needed.
 * XXX: You're responsible for completing this routine
 *
 * @c1 The total number of bytes for data storage in L1 is 2^c1
 * @b1 The size of L1's blocks in bytes: 2^b1-byte blocks.
 * @s1 The number of blocks in each set of L1: 2^s1 blocks per set.
 * @v Victim Cache's total number of blocks (blocks are of size 2^b1).
 *    v is in [0, 4].
 * @c2 The total number of bytes for data storage in L2 is 2^c2
 * @b2 The size of L2's blocks in bytes: 2^b2-byte blocks.
 * @s2 The number of blocks in each set of L2: 2^s2 blocks per set.
 * Note: c2 >= c1, b2 >= b1 and s2 >= s1.
 */
void setup_cache(uint64_t c1, uint64_t b1, uint64_t s1, uint64_t v,
                 uint64_t c2, uint64_t b2, uint64_t s2) {
    if (c2 >= c1 && b2 >= b1 && s2 >= s1 && v <= 4 && v >= 0) {
        l1 = new cache(c1, b1, s1);
        l2 = new cache(c2, b2, s2);
        MAX_VC_SIZE = v;
        DEBUG_ASSERT(vc.size() == 0);
    } else {
        throw EINVAL;
    }
}
/**
 * Subroutine that simulates the cache one trace event at a time.
 * XXX: You're responsible for completing this routine
 *
 * @type The type of event, can be READ or WRITE.
 * @arg  The target memory address
 * @p_stats Pointer to the statistics structure
 */
void cache_access(char type, uint64_t arg, cache_stats_t* p_stats) {
    block_ptr target = nullptr;
    try {
        target = l1->access(arg);
    } catch (int errno1) {
        if (errno1 == cache::ACCESS_MISS) {
            try {
                target = l2->access(arg);
            } catch (int errno2) {
                 if (errno2 == cache::ACCESS_MISS) {

                 } else {    // should not happen EVER
                     system("exit");
                 }
            }
        } else {    // should not happen EVER
            system("exit");
        }
    }
}

/**
 * Subroutine for cleaning up any outstanding memory operations and calculating overall statistics
 * such as miss rate or average access time.
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void complete_cache(cache_stats_t *p_stats) {
    delete l1;
    delete l2;
}
