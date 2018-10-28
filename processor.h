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
    long int cycle; 
    int state; // 0 for free and 1 for busy
    struct L1_cache *local_cache;
    struct pipe *pipe_to_cache;
    struct pipe *pipe_from_cache;
    char values[CHUNK][8]; //buffer to store the values as strings
    int labels[CHUNK]; //buffer to store to labels as integers (can only be 0, 1 or 2)
};

struct processor* processor_init(char *input_file, int pro_id){
	struct processor proc;
	struct L1_cache *local_cache;
	struct pipe *pipe_to_cache;
	struct pipe *pipe_from_cache;
	proc.benchmark = input_file;
	procpro_id = pro_id;
	cycle = 0; //clock 
	state = 0; //Assume the processor is free
	return &proc;
};
void processor_run(long int cycle, struct processor *proc){
	
	if(proc->state == 1){ //Check if the processor is busy
		proc->cycle++;
		return;
	}
	else{ //Processor is free
		//Check for labels and the addr 
		if(label[cycle] == 0){
			//Read instruction
		}
		else if (label[cycle] == 1){
			//Write instruction
		}
		else if (label[cycle] == 2){
			//Computation instruction
		}
		else{
			printf("Error in labels!");
			exit(1);
		}
	}
};

void processor_read(){ //This function is called the first time to read in the labels and values from the trace
	if((proc->benchmark = fopen("", "r")) == NULL){
		printf("Error opening file")
		exit(1); 
	}
	
	while(fscanf(proc->benchmark, "%d %s", proc->labels, proc->values) == 2){ 
		//Keep reading until the new line
	}

}


#endif // PROCESSOR

