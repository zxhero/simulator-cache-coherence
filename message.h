#ifndef MESSAGE
#define MESSAGE
#include<stdio.h>
//operation
#define REPLY   0x0
#define BUSUPD  0x1
#define FLUSH   0x2
#define BUSRD   0x4
#define BUSRDX  0x8
//#define LOAD    0x10
//#define STORE   0x20
//dest or src
#define BROADCAST   0xf
#define CACHE0_ID   0x1
#define CACHE1_ID   0x2
#define CACHE2_ID   0x3
#define CACHE3_ID   0x4
#define MEMORY_ID   0x5
//reply information
#define NOT_FOUND   0x1
#define FOUND       0x2
struct msg{
    long int cycle;     //time need to handle
    int operation;
    //int reply_info;
    unsigned int addr;
    //unsigned int RW_size;   //number of bytes, used in BUSUPD
    int dest;
    int src;
};
#endif // MESSAGE

