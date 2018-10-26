#ifndef PROCESSOR
#define PROCESSOR
#include<stdio.h>
#include"Cache.h"
#include"pipe.h"
#include"message.h"

struct processor{
    int pro_id;
    FILE *benchmark;
    long int cycle;
    int state;
    struct L1_cache *local_cache;
    struct pipe *pipe_to_cache;
    struct pipe *pipe_from_cache;
};

struct processor* processor_init(char *input_file, int pro_id);
void processor_run(long int cycle, struct processor* pro);
#endif // PROCESSOR

