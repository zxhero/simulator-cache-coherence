#ifndef MESSAGE
#define MESSAGE
#include<stdio.h>
//operation of bus
#define REPLY  0x1
#define BUSUPD  0x2
#define FLUSH   0x4
#define BUSRD   0x8
#define BUSRDX  0x10
#define INVALIDATION    0x100
//operation of CPU
#define LOAD    0x20
#define STORE   0x40
#define SUCCEED 0x80
//dest or src
#define BROADCAST   0x20
#define CACHE0_ID   0x1
#define CACHE1_ID   0x2
#define CACHE2_ID   0x4
#define CACHE3_ID   0x8
#define MEMORY_ID   0x10
#define PROCESSOR_ID    0x40
//reply information
#define NOT_FOUND   0x1
#define FOUND       0x2
struct msg{
    long int cycle;     //time need to handle
    int operation;
    //int reply_info;
    unsigned int addr;
    unsigned int shared_line;
    //unsigned int RW_size;   //number of bytes, used in BUSUPD
    int dest;
    int src;
};
#include"pipe.h"
void send_message(struct msg *msg, long int cycle, int operation, unsigned int shared_line, unsigned int addr, int dest, int src, struct pipe *pipe){
    msg->cycle = cycle;
    msg->dest = dest;
    msg->operation = operation;
    msg->shared_line = shared_line;
    msg->addr = addr;
    msg->src = src;
    write_pipe(pipe,msg);
}//uesd for caches and memory
#endif // MESSAGE

