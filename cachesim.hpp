#ifndef CACHESIM_HPP
#define CACHESIM_HPP

#ifdef CCOMPILER
#include <stdint.h>
#else
#include <cstdint>
#endif

#define SHOULD_NEVER_HAPPEN system("exit")
#include <deque>
#include "cache.h"

struct cache_stats_t {
    uint64_t accesses;
    uint64_t accesses_l2;
    uint64_t accesses_vc;
    uint64_t reads;
    uint64_t read_misses_l1;
    uint64_t read_misses_l2;
    uint64_t writes;
    uint64_t write_misses_l1;
    uint64_t write_misses_l2;
    uint64_t write_back_l1;
    uint64_t write_back_l2;
    uint64_t victim_hits;
    double   avg_access_time_l1;
};

void setup_cache(uint64_t c1, uint64_t b1, uint64_t s1, uint64_t v,
                 uint64_t c2, uint64_t b2, uint64_t s2);
void cache_access(char type, uint64_t arg, cache_stats_t* p_stats);
void complete_cache(cache_stats_t *p_stats);

static const uint64_t DEFAULT_C1 = 12;   /* 4KB Cache */
static const uint64_t DEFAULT_B1 = 5;    /* 32-byte blocks */
static const uint64_t DEFAULT_S1 = 3;    /* 8 blocks per set */
static const uint64_t DEFAULT_C2 = 15;   /* 32KB Cache */
static const uint64_t DEFAULT_B2 = 5;    /* 32-byte blocks */
static const uint64_t DEFAULT_S2 = 4;    /* 16 blocks per set */
static const uint64_t DEFAULT_V =  3;    /* 3 blocks in VC */

/** Argument to cache_access rw. Indicates a load */
static const char     READ = 'r';
/** Argument to cache_access rw. Indicates a store */
static const char     WRITE = 'w';

// ADDITION BELOW
typedef block_ptr (*repair)(char, uint64_t, cache_stats_t *);

static const int VC_MISS = 979;

static cache *l1;
static cache *l2;
static uint64_t MAX_VC_SIZE;
static std::deque<uint64_t> vc;

static uint64_t swap_victim(
        uint64_t target, block_ptr victim, cache_stats_t *p_stats);

static void cache_access(
        cache *cache_ptr, char type, uint64_t address,
        cache_stats_t *p_stats, repair miss_repair);

static block_ptr repair_l1_miss(
        char type, uint64_t address, cache_stats_t *p_stats);

static block_ptr repair_l2_miss(
        char type, uint64_t address, cache_stats_t *p_stats);
        
// ADDITION ABOVE

#endif /* CACHESIM_HPP */
