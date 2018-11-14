#ifndef _MESI_
#define _MESI_
#include"Cache.h"
#include"Directory.h"

void handle_msg_fromCPU_MESI(struct cache_block* block, struct msg* msg,struct L1_cache *cache. struct directory *dir){

	if(block -> status == NULL && msg -> operation == LOAD){ //Load Miss 
		printf("Load miss!\n");
		send_message(msg,msg->cycle+1,BUSRD,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus); //Send the value to the bus to request for the value
		/* Snoop */
	}
	else if(block -> status == NULL && msg -> operation == STORE){ //Write miss 
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
			  Else if none of the other caches have the valid block, they will invalidate their copies
    		*/
    	}
    }
    else if(block -> status == EXCLUSIVE){ 
    	if(msg -> operation == LOAD){ //PrRd, Exclusive
    		//Nothing happens, cache hit
    		printf("cache hit!\n"); 
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_proc); //Send the value to the proc
    	}
    	else if(msg -> operation == STORE){ //PrWr, Exclusive 
    		printf("cache hit!\n");
    		block -> status  =  MODIFY; //Status changes to modified
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_proc); //Tell the proc that the modify was successful
    	}
    }
    else if(block -> status  == MODIFY){ //
    	if(msg -> operation == LOAD){ //PrRd, Modified
    		//Nothing happens
    		printf("cache hit!\n");
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_proc); //Send the value to the proc
    	}
    	else if(msg -> operation == STORE){ //PRWr, Modified
    		//Nothing happens 
    		printf("cache hit!\n");
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_proc); //Send the value to the proc
    	}
    }
    else if(block -> status == SHARED){ 
    	if(msg -> operation ==  LOAD){ //PrRd, Shared
    		//Nothing happens
    		printf("cache hit!\n"); 
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_proc); //Send the value to the proc
    	}
    	else if(msg -> operation == STORE){ //PrWr, Shared
    		printf("cache hit!\n");
    		block -> status = MODIFY; //State changes to modfied because this cache has the latest copy
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_proc); //Send the value to the proc
    	}	
    }
    else{
    	printf("Wrong Message from Pro!\n");
    }
}

void handle_msg_fromBUS_MESI(struct cache_block* block, struct msg* msg,struct L1_cache *cache,long int cycle){
	if(block == NULL){
		printf("cache block is empty!\n");
	}
	else if(block -> status == INVALID){ 
		if(msg -> operation == BUSRDX){
			block -> status = MODIFY; //Changed to modified
			send_message(msg,msg->cycle+1,FLUSH,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus); 
		}
		//Don't have to do anything if the operation is a busread since the value is invalid
	}
	else if(block -> status == EXCLUSIVE){
		if(msg -> operation == BUSRD){ //Busread, Exclusive 
			block -> status = SHARED; //Status changes to shared after sharing the value of this cache block
			send_message(msg,msg->cycle+1,FLUSH,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus); //Send bus read
		}
		else if(msg -> operation == BUSRDX){
			block -> status = INVALID; //Status changes to invalid because there is another processor that wants to write to this cache block
			send_message(msg,msg->cycle+1,FLUSH,block,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus); //Sends flush to the bus
		}
	}
	else if(block -> status == SHARED){
		if(msg -> operation == BUSRD){
			printf("cache hit\n"); 
			send_message(msg,msg->cycle+1,FLUSH,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus);
		}
		else if(msg -> operation == BUSRDX){
			block -> status = MODIFY; //This cache has the most updated value since it is getting written to 
			send_message(msg,msg->cycle+1,FLUSH,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus); //Send bus read X
		}
	}
	else if(block -> status == MODIFY){
		if(msg -> operation == BUSRD){
			block -> status = SHARED;
			send_message(msg,msg->cycle+1,FLUSH,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus); 
			printf("cache hit!\n");
		}
		else if(msg -> operation ==BUSRDX){
			send_message(msg,msg->cycle+1,FLUSH,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus); 
			printf("cache hit!\n");
		}
	}
	else{
		printf("Wrong message from bus!\n");
	}
}
#endif // MESI
