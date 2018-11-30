#ifndef _MESI_
#define _MESI_
#include"Cache.h"
#include"Directory.h"

void handle_msg_fromCPU_MESI(struct cache_block* block, struct msg* msg,struct L1_cache *cache, struct directory *dir){

	if(block == NULL && msg -> operation == LOAD){ //Load Miss
		//printf("Load miss!\n");
		send_message(msg,msg->cycle+1,BUSRD,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus); //Send the value to the bus to request for the value
		/* Snoop */
	}
	else if(block == NULL && msg -> operation == STORE){ //Write miss
		//printf("Write miss!\n");
		send_message(msg,msg->cycle+1,BUSRDX,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus); //Send the value to the bus
		/* Snoop */
	}
    else if(block -> status == EXCLUSIVE){
        cache->access_pdata++;
    	if(msg -> operation == LOAD){ //PrRd, Exclusive
    		//Nothing happens, cache hit
    		//printf("read hit!\n");
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro); //Send the value to the proc
    	}
    	else if(msg -> operation == STORE){ //PrWr, Exclusive
    		//printf("write hit!\n");
    		block -> status  =  MODIFY; //Status changes to modified
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro); //Tell the proc that the modify was successful
    	}
    }
    else if(block -> status  == MODIFY){ //
        cache->access_pdata++;
    	if(msg -> operation == LOAD){ //PrRd, Modified
    		//Nothing happens
    		//printf("read hit!\n");
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro); //Send the value to the proc
    	}
    	else if(msg -> operation == STORE){ //PRWr, Modified
    		//Nothing happens
    		//printf("write hit!\n");
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro); //Send the value to the proc
    	}
    }
    else if(block -> status == SHARED){
        cache->access_sdata++;
    	if(msg -> operation ==  LOAD){ //PrRd, Shared
    		//Nothing happens
    		//printf("read hit!\n");
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro); //Send the value to the proc
    	}
    	else if(msg -> operation == STORE){ //PrWr, Shared
    		//printf("write hit!\n");
    		block -> status = MODIFY; //State changes to modfied because this cache has the latest copy
    		send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro); //Send the value to the proc
    		struct msg *send_msg = malloc(sizeof(struct msg));
    		int shared = check_cache_block_in_all(dir,msg->addr);
    		if(shared != cache->id)
                send_message(send_msg,msg->cycle,INVALIDATION,0,msg->addr,shared-cache->id,cache->id,cache->pipe_to_bus); // send invalid message
    	}
    }
    else{
    	//printf("Wrong Message from Pro!\n");
    }
}

void handle_msg_fromBUS_MESI(struct cache_block* block, struct msg* msg,struct L1_cache *cache,long int cycle, struct directory *dir){
	if(block == NULL){
		if((msg -> operation & REPLY) != 0 && ((msg->operation & BUSRD) != 0 || (msg->operation & BUSRDX) !=0 )){ //Reply for bus read or bus read X
            struct cache_block *new_block = find_available_block(cache, msg->addr);
            //printf("msg is reply!");
            if(new_block->status != INVALID){ //evict old cache block
                unsigned int evict_addr = new_block->addr * cache->block_size;
                //printf(", and we need to evict %x", evict_addr);

                int dest = check_cache_block_in_all(dir,evict_addr);
                if(dest == cache->id){ //None of the caches has it
                    int find = 0;
                    struct element* send_msg = NULL;
                    list_for_each_entry(send_msg,&cache->pipe_to_bus->head.head,head){
                        if((send_msg->msg->addr/cache->block_size) == (new_block->addr)){
                            find = 1;
                            break;
                        }
                    }
                    if(find == 0){  //not in msg buffer, that is the cache didn't send the data to other caches.
                        struct msg* evict_msg = calloc(1, sizeof(struct msg));
                        send_message(evict_msg,cycle+1,FLUSH,0,evict_addr,MEMORY_ID,cache->id,cache->pipe_to_bus);
                    }
                }
                //else{
                //    send_message(evict_msg,cycle+1,FLUSH,0,evict_addr,dest,cache->id,cache->pipe_to_bus);
                //}
            }
            if((msg->operation & BUSRDX) != 0){
                new_block->status = MODIFY;
                cache->access_pdata++;
            }
            else if(check_cache_block_in_all(dir,msg->addr) == 0){
                new_block->status = EXCLUSIVE;
                cache->access_pdata++;
            }
            else{
                new_block->status = SHARED;
                cache->access_sdata++;
            }
            new_block->addr = msg->addr/cache->block_size;
            send_message(msg,cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,0,cache->pipe_to_pro);
        }
        else if((msg->operation & BUSRD) != 0 || (msg->operation & BUSRDX) !=0) {           //check msg buffer
            struct element* send_msg = NULL;
            list_for_each_entry(send_msg,&cache->pipe_to_bus->head.head,head){
                if(send_msg->msg->addr/cache->block_size != msg->addr/cache->block_size) continue;
                if((send_msg->msg->operation & REPLY) != 0){
                    send_message(msg,cycle+1,REPLY|msg->operation,0,msg->addr,msg->src,cache->id,cache->pipe_to_bus);
                    break;
                }
                else if((send_msg->msg->operation & FLUSH) != 0){
                    list_delete_entry(&send_msg->head);
                    free(send_msg->msg);
                    free(send_msg);
                    send_message(msg,cycle+1,REPLY|msg->operation,0,msg->addr,msg->src,cache->id,cache->pipe_to_bus);
                    break;
                }
            }
        }
	}
	else if(block -> status == EXCLUSIVE){
		if(msg -> operation == BUSRD){ //Busread, Exclusive
            //printf("msg is BUSRD!");
			block -> status = SHARED; //Status changes to shared after sharing the value of this cache block
			send_message(msg,cycle+1,REPLY|BUSRD,0,msg->addr,msg->src,cache->id,cache->pipe_to_bus); //Send bus read
		}
		else if(msg -> operation == BUSRDX){
            //printf("msg is BUSRDX!");
			block -> status = INVALID; //Status changes to invalid because there is another processor that wants to write to this cache block
			send_message(msg,cycle+1,REPLY|BUSRDX,0,msg->addr,msg->src,cache->id,cache->pipe_to_bus); //Sends flush to the bus
		}
	}
	else if(block -> status == SHARED){
		if(msg -> operation == BUSRD){
            //printf("msg is BUSRD!");
			send_message(msg,cycle+1,REPLY|BUSRD,0,msg->addr,msg->src,cache->id,cache->pipe_to_bus);
		}
		else if(msg -> operation == BUSRDX){
            //printf("msg is BUSRDX!");
			block -> status = INVALID; //Some other cache is going to write to my data so this data is invalid
			send_message(msg,cycle+1,REPLY|BUSRDX,0,msg->addr,msg->src,cache->id,cache->pipe_to_bus); //Send bus read X
		}
		else if(msg->operation == INVALIDATION){
            //printf("msg is INVALIDATION!");
            block->status = INVALID;
		}
	}
	else if(block -> status == MODIFY){
		if(msg -> operation == BUSRD){
            //printf("msg is BUSRD!");
			block -> status = SHARED;
			send_message(msg,cycle+1,REPLY|BUSRD,0,msg->addr,msg->src,cache->id,cache->pipe_to_bus);
		}
		else if(msg -> operation ==BUSRDX){
            //printf("msg is BUSRDX!");
			block->status  = INVALID; //Some other cache is going to write ot my data so this data is invalid
			send_message(msg,cycle+1,REPLY|BUSRDX,0,msg->addr,msg->src,cache->id,cache->pipe_to_bus);
		}
	}
	else{
		//printf("Wrong message from bus!\n");
	}
    //printf("\n");
}
#endif // MESI
