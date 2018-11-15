#ifndef MEMORY
#define MEMORY
#include<stdio.h>
#include<stdlib.h>
#include"pipe.h"

//#define PORT_BUSY   1
//#define PORT_FREE   2
#define BLOCK_IN_CACHE   1
#define BLOCK_WBACK      2
#define BLOCK_SENDING    3
#define BLOCK_INDEX_MASK 0xffc

/*struct port{
    int state;
    long int busy_cycle_num;
};*/
struct mem_block{
    struct list_head head;
    unsigned int addr;
    unsigned int state;
    long int cycle;     //the cycle finishing writing back
    int     cache_id;
};
struct memory{
    /*struct port port0;
    struct port port1;
    struct port port2;
    struct port port3;*/
    struct pipe *pipe_to_bus;
    struct pipe *pipe_from_bus;
    struct mem_block *blocks_in_cache;       //record which block is in cache
    int block_size;
};

struct memory * memory_init(int block_size){
    printf("memory init....\n");
    struct memory * mem = malloc(sizeof(struct memory));
    mem->pipe_from_bus = malloc(sizeof(struct pipe));
    mem->pipe_to_bus = malloc(sizeof(struct pipe));
    init_pipe(mem->pipe_from_bus);
    init_pipe(mem->pipe_to_bus);
    mem->block_size = block_size;
    /*mem->port0.state = PORT_FREE;
    mem->port1.state = PORT_FREE;
    mem->port2.state = PORT_FREE;
    mem->port3.state = PORT_FREE;*/
    mem->blocks_in_cache = malloc(sizeof(struct mem_block));
    init_list_head(&mem->blocks_in_cache->head);
    return mem;
}

struct mem_block * lookup_mem(unsigned int addr, struct memory *mem){
    struct mem_block *entry;
    list_for_each_entry(entry,&mem->blocks_in_cache->head,head){
        if(entry->addr == addr){
            return entry;
        }else continue;
    }
    return NULL;
}

void write_back(struct memory *mem, long int cycle){
    //printf("memory WB run...\n");
    struct mem_block *entry, *next_entry;
    list_for_each_entry_safe(entry,next_entry,&mem->blocks_in_cache->head,head){
        if(entry->state == BLOCK_WBACK && entry->cycle == cycle){
            printf("memory delete %x\n",entry->addr * mem->block_size);
            list_delete_entry(&entry->head);
            free(entry);
        }else if(entry->state == BLOCK_SENDING){
            struct element *msg_entry;
            int find = 0;
            list_for_each_entry(msg_entry,&mem->pipe_to_bus->head.head,head){
                if(msg_entry->msg->dest == entry->cache_id && entry->addr == ((msg_entry->msg->addr) / mem->block_size)){
                    find = 1;                    
                    break;
                }
            }
            if(find == 0){
                entry->state = BLOCK_IN_CACHE;
            }
        }
    }
}

void memory_run(struct memory *mem, long int cycle){
    //printf("memory run...\n");
    struct element *request,*next_request;
    write_back(mem,cycle);
    list_for_each_entry_safe(request,next_request,&mem->pipe_from_bus->head.head,head){
        //printf("CHECK1\n");
        if(request->msg->cycle <= cycle){
            //printf("CHECK2\n");
            unsigned int addr = (request->msg->addr) / mem->block_size;
            struct mem_block *entry = lookup_mem(addr,mem);
            if((request->msg->operation & (BUSRD | BUSRDX)) != 0){                                      //read mem
                if(entry == NULL){
                    printf("cycle %ld, mem will send back data(BLOCK_in_mem) to cahce%d in 100 cycle.\n",cycle,request->msg->src);
                    struct msg *reply = malloc(sizeof(struct msg));
                    memset(reply,0,sizeof(struct msg));
                    //printf("CHECK2\n");
                    send_message(reply,cycle + 100,request->msg->operation | REPLY,request->msg->src,request->msg->addr,request->msg->src,MEMORY_ID,mem->pipe_to_bus);
                    //printf("CHECK4\n");
                    entry = malloc(sizeof(struct mem_block));
                    //printf("CHECK3\n");
                    entry->addr = addr;
                    entry->state = BLOCK_SENDING;
                    entry->cache_id = request->msg->src;
                    entry->cycle = cycle + 100;
                    list_add_head(&entry->head,&mem->blocks_in_cache->head);
                }else{
                    if(entry->state == BLOCK_WBACK){
                        printf("cycle %ld, mem will send back data(BLOCK_WBACK) to cahce%d in 100 cycle.\n",cycle,request->msg->src);
                        entry->state = BLOCK_SENDING;
                        entry->cache_id = request->msg->src;
                        entry->cycle = cycle + 100;
                        struct msg *reply = malloc(sizeof(struct msg));
                        memset(reply,0,sizeof(struct msg));
                        //send_message(reply,entry->cycle + 100,request->msg->operation | REPLY,request->msg->src,request->msg->addr,request->msg->src,MEMORY_ID,mem->pipe_to_bus);
                        send_message(reply,cycle + 100,request->msg->operation | REPLY,request->msg->src,request->msg->addr,request->msg->src,MEMORY_ID,mem->pipe_to_bus);
                    }else if(entry->state == BLOCK_SENDING){
                        printf("cycle %ld, mem will send back data(BLOCK_SENDING) to cahce%d in 100 cycle.\n",cycle,request->msg->src);
                        int shared = 0;
                        long int time = 0;
                        struct element *msg_entry;
                        list_for_each_entry(msg_entry,&mem->pipe_to_bus->head.head,head){
                            if(((msg_entry->msg->addr) / mem->block_size) == addr){
                                shared = msg_entry->msg->shared_line;
                                time = msg_entry->msg->cycle;
                                msg_entry->msg->shared_line |= request->msg->src;
                            }
                        }
                        struct msg *reply = malloc(sizeof(struct msg));
                        memset(reply,0,sizeof(struct msg));
                        send_message(reply,time,request->msg->operation | REPLY,request->msg->src | shared,request->msg->addr,request->msg->src,MEMORY_ID,mem->pipe_to_bus);
                    }else{
                        printf("cycle %ld, request for %x from cache%d is in cache.\n",cycle,request->msg->addr,request->msg->src);
                    }
                }
            }else if((request->msg->operation & FLUSH) != 0){                                  //write mem
                entry->state = BLOCK_WBACK;
                entry->cycle = cycle + 100;
            }else{
                exit(1);
            }
        }
        else break;
        
        list_delete_entry(&request->head);
        free(request->msg);
        free(request);
    }
    /* test */
    /*struct msg *request = peek_at_msg(mem->pipe_from_bus);
    if(request != NULL && request->cycle == cycle){
        request = read_pipe(mem->pipe_from_bus);
        request->operation = REPLY;
        request->cycle = cycle + random()%10;
        printf("cycle %ld, memory recieve requst from cache %d, and wait %ld\n",cycle, request->src,request->cycle - cycle);
        request->dest = request->src;
        request->src = MEMORY_ID;

        write_pipe(mem->pipe_to_bus, request);
    }
    return;*/
}
#endif // MEMORY

