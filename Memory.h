#ifndef MEMORY
#define MEMORY
#include<stdio.h>
#include"pipe.h"

//#define PORT_BUSY   1
//#define PORT_FREE   2
#define BLOCK_IN_CACHE   1
#define BLOCK_WBACK      2
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
};
struct memory{
    /*struct port port0;
    struct port port1;
    struct port port2;
    struct port port3;*/
    struct pipe *pipe_to_bus;
    struct pipe *pipe_from_bus;
    struct mem_block *blocks_in_cache;       //record which block is in cache
};

struct memory * memory_init(){
    struct memory * mem = malloc(sizeof(struct memory));
    mem->pipe_from_bus = malloc(sizeof(struct pipe));
    mem->pipe_to_bus = malloc(sizeof(struct pipe));
    init_pipe(mem->pipe_from_bus);
    init_pipe(mem->pipe_to_bus);
    /*mem->port0.state = PORT_FREE;
    mem->port1.state = PORT_FREE;
    mem->port2.state = PORT_FREE;
    mem->port3.state = PORT_FREE;*/
    mem->blocks_in_cache = malloc(sizeof(struct mem_block));
    init_list_head(&mem->blocks_in_cache->head);
    return mem;
}

unsigned int lookup_mem(unsigned int addr, struct memory *mem){
    struct mem_block *entry;
    list_for_each_entry(entry,&mem->blocks_in_cache->head,head){
        if(entry->addr == addr){
            return entry;
        }else continue;
    }
    return 0;
}

void write_back(struct memory *mem, long int cycle){
    struct mem_block *entry, *next_entry;
    list_for_each_entry_safe(entry,next_entry,&mem->blocks_in_cache->head,head){
        if(entry->state == BLOCK_WBACK && entry->cycle == cycle){
            list_delete_entry(&entry->head);
            free(entry);
        }
    }
}

void memory_run(struct memory *mem, long int cycle){
    struct element *request,*next_request;
    write_back(mem,cycle);
    list_for_each_entry_safe(request,next_request,&mem->pipe_from_bus->head.head,head){
        if(request->msg->cycle <= cycle){
            unsigned int addr = request->msg->addr;
            struct mem_block *entry = lookup_mem(addr,mem);
            if(request->msg->operation & (BUSRD | BUSRDX) != 0){                                      //read mem
                if(entry == 0){
                    struct msg *reply = malloc(sizeof(struct msg));
                    memset(reply,0,sizeof(struct msg));
                    reply->addr = addr;
                    reply->cycle = 100;
                    reply->dest = request->msg->src;
                    reply->src = MEMORY_ID;
                    write_pipe(mem->pipe_to_bus,reply);
                    entry = malloc(sizeof(struct mem_block));
                    entry->addr = addr;
                    entry->state = BLOCK_IN_CACHE;
                    list_add_head(&entry->head,&mem->blocks_in_cache->head);
                }else{
                    if(entry->state == BLOCK_WBACK){
                        struct msg *reply = malloc(sizeof(struct msg));
                        memset(reply,0,sizeof(struct msg));
                        reply->addr = addr;
                        reply->cycle = entry->cycle + 100;
                        reply->dest = request->msg->src;
                        reply->src = MEMORY_ID;
                        write_pipe(mem->pipe_to_bus,reply);
                    }
                }
            }else if(request->msg->operation & FLUSH != 0){                                  //write mem
                entry->state = BLOCK_WBACK;
                entry->cycle = cycle + 100;
            }else{
                exit(1);
            }
        }
        else break;
        list_delete_entry(&request->head);
        free(request);
    }

}
#endif // MEMORY

