#ifndef CACHE
#define CACHE
#include<stdio.h>
#include"pipe.h"
#include"message.h"
#include"Dragon.h"
#include"MESI.h"
#define MESI    1
#define DRAGON  2
//shared line value
#define PROCESSOR0_HAVE     0x1
#define PROCESSOR1_HAVE     0x2
#define PROCESSOR2_HAVE     0x4
#define PROCESSOR3_HAVE     0x8

struct cache_block{
    unsigned int addr;
    int status;
    unsigned int shared_line;
};
struct cache_bank{
    struct cache_block *blocks;
};
struct L1_cache{
    struct pipe *pipe_to_pro;
    struct pipe *pipe_from_pro;
    struct pipe *pipe_to_bus;
    struct pipe *pipe_from_bus;
    struct cache_bank *banks;
    unsigned int set_index_mask;
};

void cache_init(int cache_size, int associativity, int block_size, char* protocol);
void cache_run(struct L1_cache *cache, long int cycle);
int lookup_cache(struct L1_cache *cache, unsigned int addr);
#endif // CACHE
