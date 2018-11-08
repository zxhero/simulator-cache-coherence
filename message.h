#ifndef MESSAGE
#define MESSAGE
#include<stdio.h>
//operation of bus
#define REPLY   0x0
//#define REPLYX  0x1
#define BUSUPD  0x2
#define FLUSH   0x4
#define BUSRD   0x8
#define BUSRDX  0x10
//operation of CPU
#define LOAD    0x10
#define STORE   0x20
#define SUCCEED 0x40
//dest or src
#define BROADCAST   0xff
#define CACHE0_ID   0x1
#define CACHE1_ID   0x2
#define CACHE2_ID   0x4
#define CACHE3_ID   0x8
#define MEMORY_ID   0x10
#define PROCESSOR_ID    0x20
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
#endif // MESSAGE

