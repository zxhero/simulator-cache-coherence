#ifndef PIPE
#define PIPE
#include<stdio.h>
#include"message.h"

struct element{
    struct msg* msg;
    struct element* pre;
    struct element* next;
};
struct pipe{
    struct element head;
    int num;
};

void write_pipe(struct pipe* pipe, struct msg* msg){}; //arrange msg in cycle order
struct msg* read_pipe(struct pipe* pipe){};
#endif // PIPE

