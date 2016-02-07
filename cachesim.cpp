#include "cachesim.hpp"

typedef struct trace {
    std::string stat_1;
    std::string stat_2;
    std::string stat_vc;

    void reset() {
        stat_1 = "**";
        stat_2 = "**";
        stat_vc = "**";
    }

    trace() {
        reset();
    }

    std::string get_trace() {
        std::string ret = stat_1 + stat_vc + stat_2 + "\n";
        reset();
        return ret;
    }

    void hit_l1() {
        stat_1 = "H1";
    }

    void miss_l1() {
        stat_1 = "M1";
    }

    void hit_l2() {
        stat_2 = "H2";
    }

    void miss_l2() {
        stat_2 = "M2";
    }

    void hit_vc() {
        stat_vc = "HV";
    }

    void miss_vc() {
        stat_vc = "MV";
    }
} trace_t;

static cache *l1;
static cache *l2;
static uint64_t MAX_VC_SIZE;
static std::deque<uint64_t> vc;
static trace_t last;

static void cache_access(
        cache *cache_ptr, char type, uint64_t address,
        cache_stats_t *p_stats, repair miss_repair);

static block_ptr repair_l1_miss(
        char type, uint64_t address, cache_stats_t *p_stats);

static block_ptr repair_l2_miss(
        char type, uint64_t address, cache_stats_t *p_stats);


static uint64_t vc_access(uint64_t target, cache_stats_t *p_stats) {
    p_stats->accesses_vc++;
    uint64_t offset_mask = ~(l1->getMask_off());
    uint64_t masked_target = target & offset_mask;
    for (std::deque<uint64_t>::iterator iter = vc.begin(); iter != vc.end(); iter++) {
        if ((*iter & offset_mask) == masked_target) {
            // swapping and return
            p_stats->victim_hits++;
            target = *iter;
            DEBUG_ASSERT((target & ~(l1->getMask_off())) == masked_target);
            DEBUG_PRINT("[vc_access] VC_HIT %lx found\n", target);
            vc.erase(iter);
            last.hit_vc();
            return target;
        }
    }
    DEBUG_PRINT("[vc_access] VC_MISS\n");
    last.miss_vc();
    throw VC_MISS;
}

static void vc_push(uint64_t address) {
    DEBUG_ASSERT(vc.size() <= MAX_VC_SIZE);
    if (vc.size() == MAX_VC_SIZE) {
        DEBUG_PRINT("[swap_victim] VC full, burying %lx\n", vc.front());
        vc.pop_front();
    }
    DEBUG_PRINT("[swap_victim] pushing victim %lx\n", address);
    vc.push_back(address);
    DEBUG_ASSERT(vc.size() <= MAX_VC_SIZE);
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
    std::cout << last.get_trace();
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

    double ht1 = 2 + 0.2 * l1->get_s();
    double ht2 = 4 + 0.4 * l2->get_s();

    double mr1 = (p_stats->read_misses_l1 + p_stats->write_misses_l1) / ((double) p_stats->accesses);
    double mr2 = (p_stats->read_misses_l2 + p_stats->write_misses_l2) / ((double) p_stats->accesses_l2);

    double aat2 = ht2 + mr2 * 500;

    double mp1 = aat2 * (MAX_VC_SIZE == 0 ? 1 : (
            (p_stats->accesses_vc - p_stats->victim_hits) / ((double) p_stats->accesses_vc)
    ));

    p_stats->avg_access_time_l1 = ht1 + mr1 * mp1;
    printf("\n");
}

// ADDITION BELOW

static void cache_access(
        cache *cache_ptr, char type, uint64_t address,
        cache_stats_t *p_stats, repair miss_repair) {
    DEBUG_PRINT("[cache_acccess] %s access at %lx on %s\n", type == READ ? "Read" : "Write", address,
                cache_ptr == l1 ? "L1" : "L2");
    block_ptr blck = nullptr;
    try {
        blck = cache_ptr->access(address);
        if (cache_ptr == l1) {
            last.hit_l1();
        } else if (cache_ptr == l2 && type == READ) {
            last.hit_l2();
        }
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
        DEBUG_ASSERT(!(blck->valid));
    } catch (int errno_full) {
        if (errno_full == cache::SET_FULL) {
            blck = l1->find_victim_block(address);
            DEBUG_ASSERT(blck->valid);
        } else {
            SHOULD_NEVER_HAPPEN;
        }
    }
    try {
        address = vc_access(address, p_stats);
    } catch (int errno_vc) {
        if (errno_vc == VC_MISS) {
            p_stats->accesses_l2++;
            cache_access(l2, READ, address, p_stats, repair_l2_miss);
        } else {
            SHOULD_NEVER_HAPPEN;
        }
    }
    if (blck->valid) {
        if (blck->dirty) {
            p_stats->write_back_l1++;
            blck->dirty = false;
            p_stats->accesses_l2++;
            cache_access(l2, WRITE, blck->address, p_stats, repair_l2_miss);
        }
        blck->valid = false;
        vc_push(blck->address);
    }
    DEBUG_ASSERT(!(blck->valid));
    DEBUG_ASSERT(!(blck->dirty));
    l1->store_block(address, blck);
    DEBUG_ASSERT(blck->valid);
    if (type == READ) {
        p_stats->read_misses_l1++;
    } else if (type == WRITE) {
        p_stats->write_misses_l1++;
    } else {
        SHOULD_NEVER_HAPPEN;
    }
    last.miss_l1();
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
        }
        blck->valid = false;
    }
    DEBUG_ASSERT(!(blck->valid));
    DEBUG_ASSERT(!(blck->dirty));
    l2->store_block(address, blck);
    DEBUG_ASSERT(blck->valid);
    if (type == READ) {
        p_stats->read_misses_l2++;
        last.miss_l2();
    } else if (type == WRITE) {
        p_stats->write_misses_l2++;
    } else {
        SHOULD_NEVER_HAPPEN;
    }
    return blck;
}

// ADDITION ABOVE

