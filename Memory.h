#ifndef MEMORY
#define MEMORY
#include<stdio.h>
#include"pipe.h"

#define PORT_BUSY   1
#define PORT_FREE   2
#define BLOCK_DIRTY 1
#define BLOCK_CLEAN 2
#define BLOCK_INDEX_MASK 0xffc

struct port{
    int state;
    long int busy_cycle_num;
};
struct mem_block{
    unsigned int addr;
    int status;
};
struct memory{
    struct port port0, port1, port2, port3;
    struct pipe *pipe_to_bus;
    struct pipe *pipe_from_bus;
    struct mem_block *blocks;
};

void memory_init();
void memory_run(struct memory *mem, long int cycle);
#endif // MEMORY

