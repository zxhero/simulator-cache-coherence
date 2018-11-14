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
    struct directory *dir;
    cache0 = cache_init(atoi(cache_size),atoi(associativity),atoi(block_size),protocol,CACHE0_ID);
    cache1 = cache_init(atoi(cache_size),atoi(associativity),atoi(block_size),protocol,CACHE1_ID);
    cache2 = cache_init(atoi(cache_size),atoi(associativity),atoi(block_size),protocol,CACHE2_ID);
    cache3 = cache_init(atoi(cache_size),atoi(associativity),atoi(block_size),protocol,CACHE3_ID);
    dir = directory_init(cache0, cache1, cache2, cache3); 
    directory
    struct processor* pro0 = processor_init(input_file,0,cache0);
    //pro0->local_cache = cache0;
    struct processor* pro1 = processor_init(input_file,1,cache1);
    //pro1->local_cache = cache1;
    struct processor* pro2 = processor_init(input_file,2,cache2);
    //pro2->local_cache = cache2;
    struct processor* pro3 = processor_init(input_file,3,cache3);
    //pro3->local_cache = cache3;
    struct memory *mem = memory_init(atoi(block_size));
    struct bus *bus = bus_init(cache0,cache1,cache2,cache3,mem);
    Finish = 0;
    cycle = 0;
    //run the simulator
    while(pro0->state != 4 || pro1->state != 4||pro2->state != 4 ||pro3->state != 4){
        processor_run(cycle,pro0);
        cache_run(cache0,cycle, dir);
        processor_run(cycle,pro1);
        cache_run(cache1,cycle, dir);
        processor_run(cycle,pro2);
        cache_run(cache2,cycle, dir);
        processor_run(cycle,pro3);
        cache_run(cache3,cycle, dir);
        bus_run(bus,cycle);
        memory_run(mem,cycle);
        cycle++;
    }
    //print results
    printf("protocol   benchmark  cycles\n");
    printf("%s  %s  %ld\n",protocol,input_file,cycle);
    return 0;
}
