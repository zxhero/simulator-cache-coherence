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
//cache status
#define FREE    1

struct cache_block{
    unsigned int addr;
    int status;
    unsigned int shared_line;
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
};

struct L1_cache* cache_init(int cache_size, int associativity, int block_size, char* protocol){
    int i, num_of_blocks, mask = 0xfffffffff;
    struct L1_cache *local_cache = malloc(sizeof(struct L1_cache));
    local_cache->pipe_from_bus = malloc(struct pipe);
    init_pipe(local_cache->pipe_from_bus);
    local_cache->pipe_from_pro = malloc(struct pipe);
    init_pipe(local_cache->pipe_from_pro);
    local_cache->pipe_to_bus = malloc(struct pipe);
    init_pipe(local_cache->pipe_to_bus);
    local_cache->pipe_to_pro = malloc(struct pipe);
    init_pipe(local_cache->pipe_to_pro);
    local_cache->banks = calloc(associativity,sizeof(struct cache_bank));
    num_of_blocks = (cache_size >> 2)/block_size/associativity;
    local_cache->num_of_blocks = num_of_blocks;
    local_cache->num_of_banks = associativity;
    for(i =0;i < associativity;i++){
        local_cache->banks[i].blocks = calloc(num_of_blocks)
    }
    while(num_of_blocks > 1){
        mask<<1;
        num_of_blocks>>1;
    }
    mask ~= mask;
    local_cache->set_index_mask = (mask<<2)*block_size;
    local_cache->block_size = block_size;
    if(protocol[0] == 'M')  local_cache->protocol = MESI;
    else    local_cache->protocol = DRAGON;
};

struct cache_block* lookup_cache(struct L1_cache *cache, unsigned int addr){
    int set_index = ((addr & cache->set_index_mask) >> 2)/cache->block_size;
    addr = (addr>>2)/cache->block_size;
    int i = 0;
    struct cache_block *block;
    for(i = 0;i < cache->num_of_banks;i++){
        block = cache->banks[i].blocks[set_index];
        if(block.addr == addr)  return &block;
        else continue;
    }
    return NULL;
}; //return the pointer of cache block

struct cache_block* find_avaliable_block(struct L1_cache *cache, unsigned int addr){
    int set_index = ((addr & cache->set_index_mask) >> 2)/cache->block_size;
    int i = 0;
    struct cache_block *block;
    for(i = 0;i < cache->num_of_banks;i++){
        block = cache->banks[i].blocks[set_index];
        if(block.status == FREE)  return &block;
        else continue;
    }
    return cache->banks[0].blocks + set_index;
};

void cache_run(struct L1_cache *cache, long int cycle){
    struct msg* pro_msg = read_pipe(cache->pipe_from_pro);
    struct msg* bus_msg = read_pipe(cache->pipe_from_bus);
    struct msg* rply_msg = NULL;
    struct cache_block *block = NULL;
    if(pro_msg != NULL){
        block = lookup_cache(cache,pro_msg->addr);
        if(block != NULL){
            if(cache->protocol == DRAGON){
                rply_msg = handle_msg_fromCPU_dragon(block,pro_msg);
            }else{
                rply_msg = handle_msg_fromCPU_MESI(block,pro_msg);
            }
        }else{

        }
    }else if(bus_msg != NULL){
        block = lookup_cache(cache,bus_msg->addr);
        if(block != NULL){
            if(cache->protocol == DRAGON){
                rply_msg = handle_msg_fromBUS_dragon(block,bus_msg);
            }else{
                rply_msg = handle_msg_fromBUS_MESI(block,bus_msg);
            }
        }else{

        }
    }
    return;
};

void cache_run(struct L1_cache *cache, long int cycle){
	msg *message;
	unsigned int addr;
	bool found = false; //Flag to check if the address has been found


	if(peek_at_msg(L1_cache->pipe_from_pro) != NULL){ //Checking for message from processor
		message = read_pipe(L1_cache->pipe_from_pro);
		addr = message.addr;

		while(L1_cache->banks->blocks != NULL){ //Check if the cache block is empty
			if(L1_cache->banks->blocks.addr == addr){
				found = true;
				break;
			}
			L1_cache->banks->blocks++; //Move to the next cache block in the cache bank
		}

		if(found){ //Cache hit
			//We need to send a message to the processor to say that we have the data but how?
		}
		else{ //Cache miss
			//We need to send a message to the processor to say that we don't have the data but how?
		}
	}
	else if(peek_at_msg(L1_cache->pipe_from_bus) != NULL){ //Checking for message from bus
		message = read_pipe(L1_cache->pipe_from_bus);

		//Check for message operation and update accordingly? We need a status variable for the cache as well
	}
}

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
