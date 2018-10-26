#ifndef BUS
#define BUS
#include<stdio.h>
#include"pipe.h"
#include"Memory.h"
#include"Cache.h"
struct bus{
    struct pipe *pipe_to_C0;
    struct pipe *pipe_from_C0;
    struct pipe *pipe_to_C1;
    struct pipe *pipe_from_C1;
    struct pipe *pipe_to_C2;
    struct pipe *pipe_from_C2;
    struct pipe *pipe_to_C3;
    struct pipe *pipe_from_C3;
    struct pipe *pipe_to_mem;
    struct pipe *pipe_from_mem;
};

void bus_init(struct L1_cache *C0, struct L1_cache *C1, struct L1_cache *C2, struct L1_cache *C3, struct memory *mem);
void bus_run(struct bus *bus, long int cycle);
#endif // BUS

