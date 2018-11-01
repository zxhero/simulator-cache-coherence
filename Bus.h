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

struct bus * bus_init(struct L1_cache *C0, struct L1_cache *C1, struct L1_cache *C2, struct L1_cache *C3, struct memory *mem){
    struct bus *bus = malloc(sizeof(struct bus));
    bus->pipe_from_C0 = C0->pipe_to_bus;
    bus->pipe_to_C0 = C0->pipe_from_bus;
    bus->pipe_from_C1 = C1->pipe_to_bus;
    bus->pipe_to_C1 = C1->pipe_from_bus;
    bus->pipe_from_C2 = C2->pipe_to_bus;
    bus->pipe_to_C2 = C2->pipe_from_bus;
    bus->pipe_from_C3 = C3->pipe_to_bus;
    bus->pipe_to_C3 = C3->pipe_from_bus;
    bus->pipe_from_mem = mem->pipe_to_bus;
    bus->pipe_to_mem = mem->pipe_from_bus;
}

void bus_run(struct bus *bus, long int cycle);
#endif // BUS

