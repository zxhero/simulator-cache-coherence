#ifndef CACHE
#define CACHE
#include<stdio.h>
#include"message.h"
#include"pipe.h"
#define MESI    1
#define DRAGON  2
//shared line value
#define PROCESSOR0_HAVE     0x1
#define PROCESSOR1_HAVE     0x2
#define PROCESSOR2_HAVE     0x4
#define PROCESSOR3_HAVE     0x8
#define PROCESSOR0_MODIFY    0x10
#define PROCESSOR1_MODIFY    0x20
#define PROCESSOR2_MODIFY    0x40
#define PROCESSOR3_MODIFY    0x80
//cache block status
#define INVALID    0
#define MODIFY     1
#define EXCLUSIVE   2
//cache block status for MESI
#define SHARED      3
//cache block status for DRAGON
#define SHARED_CLEAN    4
#define SHARED_MODIFY   5
//cache status.
#define WORKING     0
#define PRWRMISS    1
#define PRRDMISS    2

struct cache_block{
    unsigned int addr;
    int status;
    unsigned int shared_line;
    /*In dragon protocol, low 4 bits in shared line represents whether each cache has same data,
    and 5-8 bit implies whether there is any cache modifing this data*/
};
struct cache_bank{
    struct cache_block *blocks; //array of cache blocks. Need to create based on input.
};
struct L1_cache{
    struct pipe *pipe_to_pro; //L1_cache to processor
    struct pipe *pipe_from_pro;
    struct pipe *pipe_to_bus;
    struct pipe *pipe_from_bus;
    struct cache_bank *banks;
    unsigned int set_index_mask;
    int num_of_blocks;
    int num_of_banks;
    int block_size;
    char protocol;
    char id;
    /*In dragon protocol, cache may stall for store or read miss*/
    char status;
};

struct L1_cache* cache_init(int cache_size, int associativity, int block_size, char* protocol, char id){
    printf("cache %c init....\n",'0'+id);
    int i, num_of_blocks;
    unsigned int mask = 0xffffffff;
    struct L1_cache *local_cache = malloc(sizeof(struct L1_cache));
    local_cache->id = id;
    local_cache->pipe_from_bus = malloc(sizeof(struct pipe));
    init_pipe(local_cache->pipe_from_bus);
    local_cache->pipe_from_pro = malloc(sizeof(struct pipe));
    init_pipe(local_cache->pipe_from_pro);
    local_cache->pipe_to_bus = malloc(sizeof(struct pipe));
    init_pipe(local_cache->pipe_to_bus);
    local_cache->pipe_to_pro = malloc(sizeof(struct pipe));
    init_pipe(local_cache->pipe_to_pro);
    local_cache->banks = calloc(associativity,sizeof(struct cache_bank));
    num_of_blocks = (cache_size)/block_size/associativity;
    local_cache->num_of_blocks = num_of_blocks;
    local_cache->num_of_banks = associativity;
    for(i =0;i < associativity;i++){
        local_cache->banks[i].blocks = calloc(num_of_blocks,sizeof(struct cache_block));
    }
    while(num_of_blocks > 1){
        mask <<= 1;
        num_of_blocks >>= 1 ;
    }
    mask = ~mask;
    local_cache->set_index_mask = (mask)*block_size;
    local_cache->block_size = block_size;
    if(protocol[0] == 'M')  local_cache->protocol = MESI;
    else    local_cache->protocol = DRAGON;
};

struct cache_block* lookup_cache(struct L1_cache *cache, unsigned int addr){
    int set_index = ((addr & cache->set_index_mask) )/cache->block_size;
    addr = (addr)/cache->block_size;
    int i = 0;
    struct cache_block *block;
    for(i = 0;i < cache->num_of_banks;i++){
        block = cache->banks[i].blocks+set_index;
        if(block->addr == addr && block->status != INVALID)  return block;
        else continue;
    }
    return NULL;
}; //return the pointer of cache block

struct cache_block* find_available_block(struct L1_cache *cache, unsigned int addr){
    int set_index = ((addr & cache->set_index_mask))/cache->block_size;
    int i = 0;
    struct cache_block *block;
    for(i = 0;i < cache->num_of_banks;i++){
        block = cache->banks[i].blocks+set_index;
        if(block->status == INVALID)  return block;
        else continue;
    }
    return cache->banks[rand()%cache->num_of_banks].blocks + set_index;
};

#include"Dragon.h"
#include"MESI.h"

void cache_run(struct L1_cache *cache, long int cycle){
    //printf("cache %d run...\n",cache->id);
    struct msg* pro_msg = peek_at_msg(cache->pipe_from_pro);
    struct msg* bus_msg = peek_at_msg(cache->pipe_from_bus);
    struct cache_block *block = NULL;
    if(pro_msg != NULL && pro_msg->cycle == cycle){
        printf("cycle %ld,cache %d read from pro. ",cycle, cache->id);
        pro_msg = read_pipe(cache->pipe_from_pro);
        block = lookup_cache(cache,pro_msg->addr);
            if(cache->protocol == DRAGON){
                handle_msg_fromCPU_dragon(block,pro_msg,cache);
            }else{
                handle_msg_fromCPU_MESI(block,pro_msg,cache);
            }
    }else if(bus_msg != NULL && bus_msg->cycle <= cycle){
        printf("cycle %ld, cache %d read from bus, src: %d. ",cycle, cache->id,bus_msg->src);
        bus_msg = read_pipe(cache->pipe_from_bus);
        block = lookup_cache(cache,bus_msg->addr);
            if(cache->protocol == DRAGON){
                handle_msg_fromBUS_dragon(block,bus_msg,cache,cycle);
            }else{
                handle_msg_fromBUS_MESI(block,bus_msg,cache,cycle);
            }
    }
    return;
    /*
        test*/
    //if(bus_msg != NULL) printf("cycle %ld, cache %d read from bus\n",cycle, cache->id);
    /*if(pro_msg != NULL && pro_msg->cycle == cycle){
        printf("cycle %ld,cache %d read from pro\n",cycle, cache->id);
        pro_msg = read_pipe(cache->pipe_from_pro);
        pro_msg->dest = MEMORY_ID;
        pro_msg->src = cache->id;
        pro_msg->cycle ++;
        write_pipe(cache->pipe_to_bus,pro_msg);
    }else if(bus_msg != NULL && bus_msg->cycle == cycle){
        printf("cycle %ld, cache %d read from bus\n",cycle, cache->id);
        bus_msg = read_pipe(cache->pipe_from_bus);
        bus_msg->dest = PROCESSOR_ID;
        bus_msg->src = cache->id;
        bus_msg->operation = SUCCEED;
        bus_msg->cycle ++;
        write_pipe(cache->pipe_to_pro,bus_msg);
    }
    return;*/
};

/*
This function will check the processor for a message.
Depending on whether its a cache hit or miss.
For a cache hit, it will send a message back to the processor to say that it has the data
If cache miss, then it will send a message to the bus to say that it doesn't have the data.
If there is no message from processor, the cache will check if there are any messages from bus. Bus message may be bus update, or request from other cache.

If bus update, then we need to update the status of the cache block. (write in Dragon.h)

if more than one cache has the data, the first cache will respond while the rest will not.

In 1 cycle, the cache can only send or read a message. Cannot do both.
*/
#endif // CACHE
