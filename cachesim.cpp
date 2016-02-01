#include "cachesim.hpp"

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
void setup_cache(
        uint64_t c1, uint64_t b1, uint64_t s1, uint64_t v,
        uint64_t c2, uint64_t b2, uint64_t s2) {
    if (c2 >= c1 && b2 >= b1 && s2 >= s1 && v <= 4 && v >= 0) {
        l1 = new cache((int) c1, (int) b1, (int) s1);
        l2 = new cache((int) c2, (int) b2, (int) s2);
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
void cache_access(char type, uint64_t arg, cache_stats_t *p_stats) {
    p_stats->accesses++;
    if (type == READ) {
        p_stats->reads++;
    } else if (type == WRITE) {
        p_stats->writes++;
    }
    cache_access(l1, type, arg, p_stats, repair_l1_miss);
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

static void cache_access(
        cache *cache_ptr, char type, uint64_t address,
        cache_stats_t *p_stats, repair miss_repair) {
    block_ptr blck = nullptr;
    try {
        blck = cache_ptr->access(address);
    } catch (int errno_acc) {
        if (errno_acc == cache::ACCESS_MISS) {
            blck = miss_repair(type, address, p_stats);
        } else {
            SHOULD_NEVER_HAPPEN;
        }
    }
    if (type == WRITE) {
        blck->dirty = true;
    }
}

static block_ptr repair_l1_miss(
        char type, uint64_t address, cache_stats_t *p_stats) {
    block_ptr blck = nullptr;
    try {
        blck = l1->find_empty_block(address);
    } catch (int errno_empty) {
        if (errno_empty == cache::SET_FULL) {
            blck = l1->find_victim_block(address);
        } else {
            SHOULD_NEVER_HAPPEN;
        }
        if (blck->dirty) {
            p_stats->write_back_l1++;
            blck->dirty = false;
            cache_access(l2, WRITE, blck->address, p_stats, repair_l2_miss);
        }
        blck->valid = false;
        // I think we only use VC if we have a victim
        uint64_t undead = EINVAL;
        try {
            undead = swap_victim(address, blck, p_stats);
        } catch (int errno_swap) {
            cache_access(l2, READ, address, p_stats, repair_l2_miss);
        }
        DEBUG_ASSERT(undead == address);
    }
    DEBUG_ASSERT(!(blck->valid));
    DEBUG_ASSERT(!(blck->dirty));
    *blck = {0};
    l1->store_block(address, blck);
    DEBUG_ASSERT(blck->valid);
    if (type == READ) {
        p_stats->read_misses_l1++;
    } else if (type == WRITE) {
        p_stats->write_misses_l1++;
    } else {
        SHOULD_NEVER_HAPPEN;
    }
    return blck;
}


static block_ptr repair_l2_miss(
        char type, uint64_t address, cache_stats_t *p_stats) {
    block_ptr blck = nullptr;
    try {
        blck = l2->find_empty_block(address);
    } catch (int errno_empty) {
        if (errno_empty == cache::SET_FULL) {
            blck = l2->find_victim_block(address);
        } else {
            SHOULD_NEVER_HAPPEN;
        }
        if (blck->dirty) {
            p_stats->write_back_l2++;
            blck->dirty = false;
            // TODO:WRITE BACK TO MEM
        }
        blck->valid = false;
    }
    DEBUG_ASSERT(!(blck->valid));
    DEBUG_ASSERT(!(blck->dirty));
    *blck = {0};
    l2->store_block(address, blck);
    DEBUG_ASSERT(blck->valid);
    if (type == READ) {
        p_stats->read_misses_l2++;
    } else if (type == WRITE) {
        p_stats->write_misses_l2++;
    } else {
        SHOULD_NEVER_HAPPEN;
    }
    return blck;
}


// what happen it is a HIT in VC and there are a empty block at L1,
// no need swap in this case, or this will never happen?
// start SWAPPING VC when the L1 IS FULL?
// As long as we have free block in the L1 Cache, we dont need to check VC?
static uint64_t swap_victim(
        uint64_t target, block_ptr victim, cache_stats_t *p_stats) {
    DEBUG_ASSERT(!(victim->dirty));
    target = target & ~(l1->getMask_off());
    for (int i = 0; i < vc.size(); i++) {
        if (vc[i] == target) {
            // swapping and return
            p_stats->victim_hits++;
            DEBUG_ASSERT(vc[i] & l1->getMask_off() == 0);
            uint64_t temp = vc[i];
            vc[i] = victim->address;
            DEBUG_ASSERT(temp == target);
            return temp;
            // TODO: DONT FORGET TO CALL STORE AGAIN
        }
    }
    // miss @ VC
    DEBUG_ASSERT(vc.size() <= MAX_VC_SIZE);
    if (vc.size() == MAX_VC_SIZE) {
        vc.pop_front();
    }
    DEBUG_ASSERT(victim->address & l1->getMask_off() == 0);
    vc.push_back(victim->address);
    DEBUG_ASSERT(vc.size() <= MAX_VC_SIZE);
    throw VC_MISS;
}
