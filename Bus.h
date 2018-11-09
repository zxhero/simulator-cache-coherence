#ifndef BUS
#define BUS
#include<stdio.h>
#include"pipe.h"
#include"Memory.h"
#include"Cache.h"
#include"message.h"
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

void bus_init(struct L1_cache *C0, struct L1_cache *C1, struct L1_cache *C2, struct L1_cache *C3, struct memory *mem){
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

void forward_msg(struct bus *bus, struct msg *message){
    switch(message->dest){
        case BROADCAST:
            write_pipe(bus->pipe_to_C0,message);
            write_pipe(bus->pipe_to_C1,message);
            write_pipe(bus->pipe_to_C2,message);
            write_pipe(bus->pipe_to_C3,message);
            write_pipe(bus->pipe_to_mem,message);
            break;
        case CACHE0_ID:
            write_pipe(bus->pipe_to_C0,message);
            break;
        case CACHE1_ID:
            write_pipe(bus->pipe_to_C1,message);
            break;
        case CACHE2_ID:
            write_pipe(bus->pipe_to_C2,message);
            break;
        case CACHE3_ID:
            write_pipe(bus->pipe_to_C3,message);
            break;
        case MEMORY_ID:
            write_pipe(bus->pipe_to_mem,message);
            break;
    }
}

/*
- Takes the message (from processor and memeory)and sends it to the pipe of the destination. 
- Processor has more priority than memory. 
- Each cycle the bus only processes 1 message. Can use read_pipe and write_pipe if needed.  
*/

void bus_run(struct bus *bus, long int cycle){
    struct msg *from_C0 = malloc(sizeof(struct msg)); //Message from C0
    struct msg *from_C1 = malloc(sizeof(struct msg)); //Message from C1
    struct msg *from_C2 = malloc(sizeof(struct msg)); //Message from C2
    struct msg *from_C3 = malloc(sizeof(struct msg)); //Message from C3
    struct msg *from_mem = malloc(sizeof(struct msg)); //Message from mem

    msg *message;


    /* Reading in all the messages from the pipe*/ 
    from_C0 = peek_at_msg(bus->pipe_from_C0);
    from_C1 = peek_at_msg(bus->pipe_from_C1);
    from_C2 = peek_at_msg(bus->pipe_from_C2);
    from_C3 = peek_at_msg(bus->pipe_from_C3);
    from_mem = peek_at_msg(bus->pipe_from_mem);

    if(from_C0->cycle == cycle){
        message = read_pipe(bus->pipe_from_C0);
        forward_msg(bus, message);
    }
    else if(from_C1->cycle == cycle){
        message = read_pipe(bus->pipe_from_C1);
        forward_msg(bus, message);
    }
    else if(from_C2->cycle == cycle){
        message = read_pipe(bus->pipe_from_C2);
        forward_msg(bus, message);
    }
    else if(from_C3->cycle == cycle){
        message = read_pipe(bus->pipe_from_C3);
        forward_msg(bus, message);
    }
    else if(from_mem->cycle == cycle){
        message = read_pipe(bus->pipe_from_mem);
        forward_msg(bus, message);
    }
};

#endif // BUS

