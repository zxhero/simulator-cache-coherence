#ifndef PROCESSOR
#define PROCESSOR
#define CHUNK 1024 /* Assume max number of instructions in trace is 1024*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"Cache.h"
#include"pipe.h"
#include"message.h"


struct processor{
    int pro_id;
    FILE *benchmark;
    long int cycle; //To set when the processor is free again when it is stalled
    int state; // 0 for free and 1 for wait for memory and 2 for wait for computation and 4 for end of work
    struct L1_cache *local_cache;
    struct pipe *pipe_to_cache;
    struct pipe *pipe_from_cache;
    long int excution_cycle;
    long int compute_cycle;
    long int num_of_LS;
    long int idle_cycle;
};

struct processor* processor_init(char *input_file, int pro_id,struct L1_cache *local_cache){
    //printf("processor %d init....\n",pro_id);
	struct processor *proc = malloc(sizeof(struct processor));
    char benchmark[100];
    char benchid[10] = "_*.data";
    strcpy(benchmark,input_file);
    benchid[1] = '0'+pro_id;
    strcat(benchmark,benchid);
	proc->pipe_from_cache = local_cache->pipe_to_pro ;
	proc->pipe_to_cache = local_cache->pipe_from_pro ;
	proc->benchmark = fopen(benchmark,"r");
    //printf("benchmark: %s\n",benchmark);
	proc->pro_id = pro_id;
	proc->cycle = 0; //clock
	proc->state = 0; //Assume the processor is free
	proc->compute_cycle = 0;
	proc->excution_cycle = 0;
	proc->idle_cycle = 0;
	proc->num_of_LS = 0;
	return proc;
};

void processor_run(long int cycle, struct processor *proc){
    //printf("processor %d run...\n",proc->pro_id);
	unsigned int addr; //To store the value of the address
	int label; //To store the value of the label
	if(proc->state == 1 && peek_at_msg(proc->pipe_from_cache) != NULL){ //There is a message from the cache
		struct msg * rply = read_pipe(proc->pipe_from_cache);
        //printf("cycle %ld, processor %d get 0x%x\n",cycle,1<<proc->pro_id,rply->addr);
		proc->state = 0; //Make the processor free again
        free(rply);
        proc->excution_cycle ++;
        proc->idle_cycle ++;
		return;
	}
	else if(proc->state == 2 && proc->cycle == cycle){ //Check if the processor is stalling for computation
		proc->state = 0; //Make the processor free again
		return;
	}
	else if(proc->state == 0){ //Processor is free & check for labels and the addr
		//proc->cycle++; //Increase the proc cycle
        proc->excution_cycle++;
		if(fscanf(proc->benchmark, "%d %x", &label, &addr) == EOF){
            proc->state = 4;
            printf("cycle %ld, processor %d end...\n",cycle,1<<proc->pro_id);
            int i;
            scanf("%d",&i);
            return;
        }; //Scans in the values and the labels
        //printf("cycle %ld, processor %d read 0x%x\n",cycle,1<<proc->pro_id,addr);
		struct msg *message = malloc(sizeof(struct msg)); //Creates a new message
		memset(message,0,sizeof(struct msg));
		if(label == 0){ //Load instruction
            proc->state = 1;
            proc->cycle = cycle;
			message->operation = LOAD;
			message->cycle = cycle+1;
			message->addr = addr;
			proc->num_of_LS ++;
			write_pipe(proc->pipe_to_cache, message); //Passes the messsage to the pipe to be sent to the cache
		}
		else if(label == 1){ //Store instruction
            proc->state = 1;
            proc->cycle = cycle;
			message->operation = STORE;
			message->cycle = cycle+1;
			message->addr = addr;
			proc->num_of_LS ++;
			write_pipe(proc->pipe_to_cache, message); //Passes the messsage to the pipe to be sent to the cache
		}
		else if(label == 2) { //Computation instruction
            proc->state = 2;
			proc->cycle = cycle + addr;
			proc->compute_cycle += addr;
			proc->excution_cycle+= addr;
			return;
		}
		else{
			//printf("error in labels!");
			exit(1);
		}

	}else if(proc->state == 1 && (proc->cycle + 500) == cycle){
        //printf("something wrong with processor %d \n",1<<proc->pro_id);
        exit(1);
    }else if(proc->state == 1){
        proc->idle_cycle++;
        proc->excution_cycle++;
	}
};
#endif // PROCESSOR

