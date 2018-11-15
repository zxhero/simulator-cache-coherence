#ifndef _DRAGON_
#define _DRAGON_
#include "Cache.h"

void handle_msg_fromCPU_dragon(struct cache_block* block, struct msg* msg, struct L1_cache *cache){
    if(msg->operation == LOAD){                     //load
        //printf("LOAD!\n");
        if(block == NULL){                          //load miss, change cache status to PrRdMiss, send busrd
            printf("Load miss! \n");
            cache->status = PRRDMISS;
            send_message(msg,msg->cycle+1,BUSRD,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus);
        }else{                                      //load hit, send back data to Pro
            printf("Load hit! \n");
            send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro);
        }
    }else if(msg->operation == STORE){                                 //store
        //printf("STORE!\n");
        if(block == NULL){                          /*store miss, change cache status to PrWrMiss, send busrd */
            printf("Store miss! \n");
            cache->status = PRWRMISS;
            send_message(msg,msg->cycle+1,BUSRD,0,msg->addr,BROADCAST,cache->id,cache->pipe_to_bus);
        }else{                                      //store hit
            printf("Store hit! status is %d \n",block->status);
            int shared = ((block->shared_line & 0xf) - cache->id);
            if(block->status == EXCLUSIVE){         //change cache block status to M, change shared line
                block->status = MODIFY;
                block->shared_line = ((block->shared_line & 0xf) | (cache->id << 4));
            }else if(block->status == SHARED_CLEAN){    //change cache block status to SM or M, send busupd, change shared line
                block->shared_line = ((block->shared_line & 0xf) | (cache->id << 4));
                if(shared != 0){
                    block->status = SHARED_MODIFY;
                    struct msg* busupd_msg = calloc(1,sizeof(struct msg));
                    send_message(busupd_msg,msg->cycle+1,BUSUPD,block->shared_line,msg->addr,shared,cache->id,cache->pipe_to_bus);
                }else{
                    block->status = MODIFY;
                }
            }else if(block->status == SHARED_MODIFY){   //send busupd, maybe change cache block status to M
                if(shared != 0){
                    struct msg* busupd_msg = calloc(1,sizeof(struct msg));
                    send_message(busupd_msg,msg->cycle+1,BUSUPD,block->shared_line,msg->addr,shared,cache->id,cache->pipe_to_bus);
                }else{
                    block->status = MODIFY;
                }
            }
            send_message(msg,msg->cycle+1,SUCCEED,0,msg->addr,PROCESSOR_ID,cache->id,cache->pipe_to_pro);
        }
    }else{
        printf("Wrong message from Pro\n");
    }
};

void handle_msg_fromBUS_dragon(struct cache_block* block, struct msg* msg, struct L1_cache *cache,long int cycle){
    if((msg->operation & REPLY) != 0 && (msg->operation & BUSRD) != 0){         //reply for busrd
        struct cache_block *new_block = find_available_block(cache,msg->addr);
        printf("msg is Reply");
        if(new_block->status != INVALID){           //evict old cache block
            unsigned int evict_addr = new_block->addr * cache->block_size;
            printf(", and we need to evict %x, status: %d", evict_addr, new_block->status);
            struct msg* evict_msg = calloc(1,sizeof(struct msg));
            if((new_block->shared_line & 0xf0) == (cache->id << 4)){        //this cache modifies it
                new_block->shared_line &= 0xf;
            }
            new_block->shared_line -= cache->id;
            if(new_block->shared_line == 0){                                //need to write back to mem
                send_message(evict_msg,cycle+1,FLUSH,0,evict_addr,MEMORY_ID,cache->id,cache->pipe_to_bus);
            }else{                                                          //only inform other shared caches
                send_message(evict_msg,cycle+1,FLUSH,new_block->shared_line,evict_addr,new_block->shared_line & 0xf,cache->id,cache->pipe_to_bus);
            }
        }
        int shared = (msg->shared_line & 0xf) - cache->id;
        if(cache->status == PRWRMISS){              //change cache status to working, change cache block status to SM or M, may send busupd, change shared line
            cache->status = WORKING;
            new_block->addr = (msg->addr ) / cache->block_size;
            new_block->shared_line = (msg->shared_line & 0xf) | (cache->id<<4);
            if(shared == 0){
                new_block->status = MODIFY;
            }else{
                new_block->status = SHARED_MODIFY;
                struct msg* busupd_msg = calloc(1,sizeof(struct msg));
                send_message(busupd_msg,cycle+1,BUSUPD,new_block->shared_line,msg->addr,shared,cache->id,cache->pipe_to_bus);
            }
        }else if(cache->status == PRRDMISS){        //change cache status to working, change cache block status to E or Sc, change shared line
            cache->status = WORKING;
            new_block->shared_line = msg->shared_line;
            new_block->addr = (msg->addr ) / cache->block_size;
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
        printf("\n");
    }else if((msg->operation & BUSUPD) != 0){         //
        printf("msg is BUSUPD\n");
        if(block->status == SHARED_CLEAN){          //change shared line
            block->shared_line = msg->shared_line;
        }else if(block->status == SHARED_MODIFY){   //change cache block status to Sc, change shared line
            struct element* send_msg = NULL;
            int find = 0;
            list_for_each_entry(send_msg,&cache->pipe_to_bus->head.head,head){
                if(send_msg->msg->operation == BUSUPD && (send_msg->msg->addr/cache->block_size) == (msg->addr/cache->block_size)){
                    find = 1;
                }
            }
            if(find == 0){
                block->status = SHARED_CLEAN;
                block->shared_line = msg->shared_line;
            }
        }
        free(msg);
    }else if((msg->operation & BUSRD) != 0){
        printf("msg is BUSRD");
        if(block != NULL){
            printf(", and this cache has it! ");
            if(block->status == EXCLUSIVE){             //change cache block status to Sc, may send reply, change shared line
                printf("reply.\n");
                block->status = SHARED_CLEAN;
                block->shared_line |= msg->src;
                send_message(msg,cycle+1,msg->operation | REPLY,block->shared_line,msg->addr,msg->src,cache->id,cache->pipe_to_bus);
            }else if(block->status == SHARED_CLEAN){    //may send reply, change shared line
                int shared = (block->shared_line & 0xf) - cache->id;
                printf(" %x %x ",block->shared_line & 0xf0, shared);
                block->shared_line |= msg->src;
                if((block->shared_line & 0xf0) == 0 &&  shared< cache->id){  //the cache withe bigger id is responsible to reply
                    printf("reply.\n");
                    send_message(msg,cycle+1,msg->operation | REPLY,block->shared_line,msg->addr,msg->src,cache->id,cache->pipe_to_bus);
                }else{
                    struct element* send_msg = NULL;
                    int find = 0;
                    list_for_each_entry(send_msg,&cache->pipe_to_bus->head.head,head){
                        if((send_msg->msg->operation & REPLY) != 0 && (send_msg->msg->addr/cache->block_size) == (msg->addr/cache->block_size)){
                            send_msg->msg->shared_line = block->shared_line;                            
                            find = 1;
                        }
                    }                    
                    if(find == 1){
                        printf("reply.\n");
                        send_message(msg,cycle+1,msg->operation | REPLY,block->shared_line,msg->addr,msg->src,cache->id,cache->pipe_to_bus);
                    }else{                      
                        printf("\n");
                        free(msg);
                    }
                }
            }else if(block->status == SHARED_MODIFY){   //send reply, change shared line
                printf("reply.\n");
                block->shared_line |= msg->src;
                send_message(msg,cycle+1,msg->operation | REPLY,block->shared_line,msg->addr,msg->src,cache->id,cache->pipe_to_bus);
            }else if(block->status == MODIFY){          //change cache block status to Sm, send reply, change shared line
                printf("reply.\n");
                block->status = SHARED_MODIFY;
                block->shared_line |= msg->src;
                send_message(msg,cycle+1,msg->operation | REPLY,block->shared_line,msg->addr,msg->src,cache->id,cache->pipe_to_bus);
            }
        }else{
            printf("\n");
            free(msg);
        }
    }else if((msg->operation & FLUSH) != 0){              //change shared line
        printf("msg is FLUSH\n");
        show_cache(cache);
        if(block == NULL){
            struct msg *send_msg = peek_at_msg(cache->pipe_to_bus);
            if(send_msg->addr == msg->addr){
                
                if((send_msg->shared_line & 0xf0) == (msg->src << 4)){
                    send_msg->shared_line = msg->shared_line - cache->id;
                }else{
                    send_msg->shared_line -= msg->src;
                }
                if(send_msg->shared_line == 0){
                    send_msg->dest = MEMORY_ID;
                }else{
                    send_msg->dest = send_msg->shared_line & 0xf;
                }
            }else{
                printf("WRong!\n");
                exit(1);
            }
        }else{
            
            block->shared_line = msg->shared_line;
        }
        free(msg);
    }
};
#endif // DRAGON
