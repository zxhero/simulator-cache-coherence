#ifndef _MESI_
#define _MESI_
#include"Cache.h"
#include"Directory.h"

void handle_msg_fromCPU_MESI(struct cache_block* block, struct msg* msg,struct L1_cache *cache, struct directory *dir){

	if(block == NULL && msg -> operation == LOAD){ //Load Miss 
		printf("Load miss!\n");
		send_message(msg,msg->cycle+1,BUSRD,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus); //Send the value to the bus to request for the value
		/* Snoop */
	}
	else if(block == NULL && msg -> operation == STORE){ //Write miss 
		printf("Write miss!\n");
		send_message(msg,msg->cycle+1,BUSRD,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus); //Send the value to the bus
		/* Snoop */
	}
    else if(block -> status == INVALID){
    	if(msg->operation == LOAD){ //PrRD, Invalid cache block
    		send_message(msg,msg->cycle+1,BUSRD,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus); //Send bus read
    		if(check_cache_block_in_all(dir, block) == 0){
    			block -> status = EXCLUSIVE;
    		}
    		else{
    			block -> status = SHARED;
    		}
    		/*Wait and read all messages from all other caches. 
    		  If one of the other caches have the valid block, then the state is changed to shared.
			  Else if none of the other caches have the valid block, then the state is changed to Exclusive.  
    		*/
    	}
    	else if(msg -> operation == STORE){ //PrWr, Invalid cache block
    		send_message(msg,msg->cycle+1,BUSRDX,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus); //Send bus readx
    		block -> status = MODIFY; //State is changed to modified
    		/*Wait and read all messages from all other caches. 
    		  If one of the other caches have the valid block, then they will send the value otherwise they will fetch from main memory 
    		*/
    	}
    }
    else if(block -> status == EXCLUSIVE){ 
    	if(msg -> operation == LOAD){ //PrRd, Exclusive
    		//Nothing happens, cache hit
    		printf("cache hit!\n"); 
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro); //Send the value to the proc
    	}
    	else if(msg -> operation == STORE){ //PrWr, Exclusive 
    		printf("cache hit!\n");
    		block -> status  =  MODIFY; //Status changes to modified
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro); //Tell the proc that the modify was successful
    	}
    }
    else if(block -> status  == MODIFY){ //
    	if(msg -> operation == LOAD){ //PrRd, Modified
    		//Nothing happens
    		printf("cache hit!\n");
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro); //Send the value to the proc
    	}
    	else if(msg -> operation == STORE){ //PRWr, Modified
    		//Nothing happens 
    		printf("cache hit!\n");
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro); //Send the value to the proc
    	}
    }
    else if(block -> status == SHARED){ 
    	if(msg -> operation ==  LOAD){ //PrRd, Shared
    		//Nothing happens
    		printf("cache hit!\n"); 
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro); //Send the value to the proc
    	}
    	else if(msg -> operation == STORE){ //PrWr, Shared
    		printf("cache hit!\n");
    		block -> status = MODIFY; //State changes to modfied because this cache has the latest copy
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro); //Send the value to the proc
    	}	
    }
    else{
    	printf("Wrong Message from Pro!\n");
    }
}

void handle_msg_fromBUS_MESI(struct cache_block* block, struct msg* msg,struct L1_cache *cache,long int cycle, struct directory *dir){
	if(block == NULL){
		printf("cache block is empty!\n");
	}
	else if((msg -> operation & REPLY) != 0 && ((msg->operation & BUSRD) != 0 || (msg->operation & BUSRDX) !=0 )){ //Reply for bus read or bus read X
		struct cache_block *new_block = find_available_block(cache, msg->addr);
		printf("msg is reply!\n");
		if(new_block->status != INVALID){ //evict old cache block
			unsigned int evict_addr = new_block->addr * cache->block_size;
			printf(", and we need to evict %x", evict_addr);
			struct msg* evict_msg = calloc(1, sizeof(struct msg));
			int c0 = check_cache_0(dir, block);
			int c1 = check_cache_1(dir, block);
			int c2 = check_cache_2(dir, block);
			int c3 = check_cache_3(dir, block);
			int dest = c0 | (c1<<1) | (c2 <<2) | (c3 << 3); 
			if(dest == 0){ //None of the caches has it 
				send_message(evict_msg,cycle+1,FLUSH,0,evict_addr,MEMORY_ID,cache->id,cache->pipe_to_bus);
			}
			else{
				send_message(evict_msg,cycle+1,FLUSH,0,evict_addr,dest,cache->id,cache->pipe_to_bus);
			}
		}
	}
	else if(block -> status == INVALID){ 
		printf("Do nothing!\n");
		//Don't have to do anything if the operation is a busread since the value is invalid
	}
	else if(block -> status == EXCLUSIVE){
		if(msg -> operation == BUSRD){ //Busread, Exclusive 
			block -> status = SHARED; //Status changes to shared after sharing the value of this cache block
			send_message(msg,msg->cycle+1,REPLY,0,msg->addr,msg->src,cache->id,cache->pipe_to_bus); //Send bus read
		}
		else if(msg -> operation == BUSRDX){
			block -> status = INVALID; //Status changes to invalid because there is another processor that wants to write to this cache block
			send_message(msg,msg->cycle+1,REPLY,0,msg->addr,msg->src,cache->id,cache->pipe_to_bus); //Sends flush to the bus
		}
	}
	else if(block -> status == SHARED){
		if(msg -> operation == BUSRD){
			send_message(msg,msg->cycle+1,REPLY,0,msg->addr,msg->src,cache->id,cache->pipe_to_bus);
		}
		else if(msg -> operation == BUSRDX){
			block -> status = INVALID; //Some other cache is going to write to my data so this data is invalid
			send_message(msg,msg->cycle+1,REPLY,0,msg->addr,msg->src,cache->id,cache->pipe_to_bus); //Send bus read X
		}
	}
	else if(block -> status == MODIFY){
		if(msg -> operation == BUSRD){
			block -> status = SHARED;
			send_message(msg,msg->cycle+1,REPLY,0,msg->addr,msg->src,cache->id,cache->pipe_to_bus); 
		}
		else if(msg -> operation ==BUSRDX){
			block->status  = INVALID; //Some other cache is going to write ot my data so this data is invalid
			send_message(msg,msg->cycle+1,REPLY,0,msg->addr,msg->src,cache->id,cache->pipe_to_bus); 
		}
	}
	else{
		printf("Wrong message from bus!\n");
	}
}
#endif // MESI
