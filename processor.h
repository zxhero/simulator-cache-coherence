#ifndef PROCESSOR
#define PROCESSOR
#define CHUNK 1024 /* Assume max number of instructions in trace is 1024*/
#include<stdio.h>
#include<stdlib.h>
#include"Cache.h"
#include"pipe.h"
#include"message.h"


struct processor{
    int pro_id;
    FILE *benchmark;
    long int cycle; //To set when the processor is free again when it is stalled 
    int state; // 0 for free and 1 for wait for memory and 2 for wait for computation
    struct L1_cache *local_cache;
    struct pipe *pipe_to_cache;
    struct pipe *pipe_from_cache;
};

struct processor* processor_init(char *input_file, int pro_id,struct L1_cache *local_cache){
	struct processor *proc = malloc(sizeof(struct processor));
	local_cache->pipe_to_pro = proc->pipe_from_cache;
	local_cache->pipe_from_pro = proc->pipe_to_cache;
	proc->benchmark = fopen(input_file,"r");
	proc->pro_id = pro_id;
	proc->cycle = 0; //clock
	proc->state = 0; //Assume the processor is free
	return proc;
};

void processor_run(long int cycle, struct processor *proc){

	int addr; //To store the value of the address 
	int label; //To store the value of the label
	if(proc->state == 1 && peek_at_msg(proc->pipe_from_cache) != NULL){ //There is a message from the cache
		read_pipe(proc->pipe_from_cache);
		proc->cycle++;
		proc->state = 0; //Make the processor free again
		return;
	}
	else if(proc->state == 2 && proc->cycle == cycle){ //Check if the processor is stalling for computation
		proc->cycle++;
		proc->state = 0; //Make the processor free again
		return;
	}
	else if(proc->state != 0){ //Processor is busy stalling 
		proc->cycle++;
		return;
	}	
	else{ //Processor is free & check for labels and the addr
		proc->cycle++; //Increase the proc cycle

		fscanf(proc->benchmark, "%d %x", &label, &addr); //Scans in the values and the labels
		struct msg *message = malloc(sizeof(struct msg)); //Creates a new message
		if(label == 0){ //Load instruction
			message->operation = LOAD;
			message->cycle = cycle+1;
			message->addr = addr;
			write_pipe(proc->pipe_to_cache, message); //Passes the messsage to the pipe to be sent to the cache 
		}
		else if(label == 1){ //Store instruction
			message->operation = STORE;
			message->cycle = cycle+1;
			message->addr = addr;
			write_pipe(proc->pipe_to_cache, message); //Passes the messsage to the pipe to be sent to the cache 
		} 
		else if(label == 2) { //Computation instruction 
			proc->cycle = cycle + addr;
			proc->cycle++;
			return;
		}
		else{
			printf("error in labels!");
			exit(1);
		}
		
	}
};
#endif // PROCESSOR

