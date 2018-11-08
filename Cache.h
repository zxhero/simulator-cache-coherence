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
    struct cache_block *blocks; //array of cache blocks. Need to create based on input. 
};
struct L1_cache{
    struct pipe *pipe_to_pro; //L1_cache to processor 
    struct pipe *pipe_from_pro;
    struct pipe *pipe_to_bus;
    struct pipe *pipe_from_bus;
    struct cache_bank *banks;
    unsigned int set_index_mask;
    int block_size;
    int associativity;
    char* protocol;
};

void cache_init(int cache_size, int associativity, int block_size, char* protocol){//Create the L1_cache
	struct L1_cache *L1_cache = malloc(sizeof(cache_size)); //Allocates space based on the cache size
	init_pipe(L1_cache->pipe_to_pro);
	init_pipe(L1_cache->pipe_from_pro);
	init_pipe(L1_cache->pipe_to_bus);
	init_pipe(L1_cache->pipe_from_bus);
	init_pipe(L1_cache->banks);
	L1_cache.block_size = block_size; 
	L1_cache.associativity = associativity;
	L1_cache.protocol = protocol; 
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

int lookup_cache(struct L1_cache *cache, unsigned int addr); //To check if the address is in the cache block
#endif // CACHE
