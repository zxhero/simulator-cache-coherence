#ifndef _DRAGON_
#define _DRAGON_
#include "Cache.h"

void handle_msg_fromCPU_dragon(struct cache_block* block, struct msg* msg, struct L1_cache *cache){
    if(msg->operation == LOAD){                     //load
        if(block == NULL){                          //load miss, change cache status to PrRdMiss, send busrd
            cache->status = PRRDMISS;
            send_message(msg,msg->cycle++,BUSRD,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus);
        }else{                                      //load hit, send back data to Pro
            send_message(msg,msg->cycle++,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro);
        }
    }else if(msg->operation == STORE){                                 //store
        if(block == NULL){                          /*store miss, change cache status to PrWrMiss, send busrd */
            cache->status = PRWRMISS;
            send_message(msg,msg->cycle++,BUSRD,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus);
        }else{                                      //store hit
            int shared = ((block->shared_line & 0xf) - cache->id);
            if(block->status == EXCLUSIVE){         //change cache block status to M, change shared line
                block->status = MODIFY;
                block->shared_line = (block->shared_line | (cache->id << 4));
            }else if(block->status == SHARED_CLEAN){    //change cache block status to SM or M, send busupd, change shared line
                block->shared_line = (block->shared_line | (cache->id << 4));
                if(shared != 0){
                    block->status = SHARED_MODIFY;
                    send_message(msg,msg->cycle++,BUSUPD,block->shared_line,msg->addr,shared,cache->id,cache->pipe_to_bus);
                }else{
                    block->status = MODIFY;
                }
            }else if(block->status == SHARED_MODIFY){   //send busupd, maybe change cache block status to M
                if(shared != 0){
                    send_message(msg,msg->cycle++,BUSUPD,block->shared_line,msg->addr,shared,cache->id,cache->pipe_to_bus);
                }else{
                    block->status = MODIFY;
                }
            }
            send_message(msg,msg->cycle++,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro);
        }
    }else{
        printf("Wrong message from Pro\n");
    }
};

void handle_msg_fromBUS_dragon(struct cache_block* block, struct msg* msg, struct L1_cache *cache,long int cycle){
    if((msg->operation & REPLY) != 0 && (msg->operation & BUSRD) != 0){         //reply for busrd
        struct cache_block *new_block = find_avaliable_block(cache,msg->addr);
        if(new_block->status != INVALID){           //evict old cache block
            struct msg* evict_msg = calloc(1,sizeof(struct msg));
            if((new_block->shared_line & 0xf0) == (cache->id << 4)){        //this cache modifies it
                new_block->shared_line &= 0xf;
            }
            new_block->shared_line -= cache->id;
            if(new_block->shared_line == 0){                                //need to write back to mem
                send_message(evict_msg,cycle+1,FLUSH,0,new_block->addr,MEMORY_ID,cache->id,cache->pipe_to_bus);
            }else{                                                          //only inform other shared caches
                send_message(evict_msg,cycle+1,FLUSH,new_block->shared_line,new_block->addr,new_block->shared_line,cache->id,cache->pipe_to_bus);
            }
        }
        int shared = (msg->shared_line & 0xf) - cache->id;
        if(cache->status == PRWRMISS){              //change cache status to working, change cache block status to SM or M, may send busupd, change shared line
            cache->status = WORKING;
            new_block->addr = msg->addr;
            new_block->shared_line = (msg->shared_line & 0xf) | (cache->id<<4);
            if(shared == 0){
                new_block->status = MODIFY;
            }else{
                new_block->status = SHARED_MODIFY;
                struct msg* busupd_msg = calloc(1,sizeof(struct msg));
                send_message(busupd_msg,cycle+1,BUSUPD,new_block->shared_line,new_block->addr,shared,cache->id,cache->pipe_to_bus);
            }
        }else if(cache->status == PRRDMISS){        //change cache status to working, change cache block status to E or Sc, change shared line
            cache->status = WORKING;
            new_block->shared_line = msg->shared_line;
            new_block->addr = msg->addr;
            if(shared == 0){
                new_block->status = EXCLUSIVE;
            }else{
                new_block->status = SHARED_CLEAN;
            }
        }else{
            printf("wrong cache status\n");
            exit(1);
        }
        send_message(msg,cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,0,cache->pipe_to_pro);
    }else if((msg->operation & BUSUPD) != 0){         //
        if(block->status == SHARED_CLEAN){          //change shared line
            block->shared_line = msg->shared_line;
        }else if(block->status == SHARED_MODIFY){   //change cache block status to Sc, change shared line
            block->status = SHARED_CLEAN;
            block->shared_line = msg->shared_line;
        }
        free(msg);
    }else if((msg->operation & BUSRD) != 0){
        if(block != NULL){
            if(block->status == EXCLUSIVE){             //change cache block status to Sc, may send reply, change shared line
                block->status = SHARED_CLEAN;
                block->shared_line |= msg->src;
                send_message(msg,cycle+1,msg->operation | REPLY,block->shared_line,block->addr,msg->src,cache->id,cache->pipe_to_bus);
            }else if(block->status == SHARED_CLEAN){    //may send reply, change shared line
                int shared = (block->shared_line & 0xf) - cache->id;
                block->shared_line |= msg->src;
                if((block->shared_line & 0xf0) == 0 &&  shared< cache->id){  //the cache withe bigger id is responsible to reply
                    send_message(msg,cycle+1,msg->operation | REPLY,block->shared_line,block->addr,msg->src,cache->id,cache->pipe_to_bus);
                }else{
                    free(msg);
                }
            }else if(block->status == SHARED_MODIFY){   //send reply, change shared line
                block->shared_line |= msg->src;
                send_message(msg,cycle+1,msg->operation | REPLY,block->shared_line,block->addr,msg->src,cache->id,cache->pipe_to_bus);
            }else if(block->status == MODIFY){          //change cache block status to Sm, send reply, change shared line
                block->status = SHARED_MODIFY;
                block->shared_line |= msg->src;
                send_message(msg,cycle+1,msg->operation | REPLY,block->shared_line,block->addr,msg->src,cache->id,cache->pipe_to_bus);
            }
        }else{
            free(msg);
        }
    }else if((msg->operation & FLUSH) != 0){              //change shared line
        block->shared_line = msg->shared_line;
        free(msg);
    }
};
#endif // DRAGON
