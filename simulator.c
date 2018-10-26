#include"simulator.h"

int main(int argc, char *argv[]){
    char *protocol, *input_file, *cache_size, *associativity, *block_size;
    if(argc == 6){
        protocol = argv[1];
        input_file = argv[2];
        cache_size = argv[3];
        associativity = argv[4];
        block_size = argv[5];
    }else{
        printf("Input command line wrong!\n");
        return 0;
    }
    //initialize
    struct L1_cache *cache0, *cache1, *cache2, *cache3;
    cache0 = cache_init(atoi(cache_size),atoi(associativity),atoi(block_size),protocol);
    cache1 = cache_init(atoi(cache_size),atoi(associativity),atoi(block_size),protocol);
    cache2 = cache_init(atoi(cache_size),atoi(associativity),atoi(block_size),protocol);
    cache3 = cache_init(atoi(cache_size),atoi(associativity),atoi(block_size),protocol);
    struct processor* pro0 = processor_init(input_file,0);
    pro0->local_cache = cache0;
    struct processor* pro1 = processor_init(input_file,1);
    pro1->local_cache = cache1;
    struct processor* pro2 = processor_init(input_file,2);
    pro2->local_cache = cache2;
    struct processor* pro3 = processor_init(input_file,3);
    pro3->local_cache = cache3;
    struct memory *mem = memory_init();
    struct bus *bus = bus_init(cache0,cache1,cache2,cache3,mem);
    Finish = 0;
    cycle = 0;
    //run the simulator
    while(Finish == 0){
        processor_run(cycle,pro0);
        cache_run(cache0,cycle);
        processor_run(cycle,pro1);
        cache_run(cache1,cycle);
        processor_run(cycle,pro2);
        cache_run(cache2,cycle);
        processor_run(cycle,pro3);
        cache_run(cache3,cycle);
        bus_run(cycle);
        memory_run(mem,cycle);
        cycle++;
    }
    //print results
    printf("protocol   benchmark  cycles\n");
    printf("%s  %s  %d\n",protocol,input_file,cycle);
    return 0;
}
