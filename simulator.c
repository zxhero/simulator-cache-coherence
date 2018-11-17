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
        memory_run(mem,cycle);
        bus_run(bus,cycle);
        cycle++;
    }
    //print results
    printf("protocol |  benchmark | total cycles  |  data traffic  |  invalidation  |  update\n");
    printf("%s  %s  %ld  %ld  %ld  %ld\n",protocol,input_file,cycle,bus->traffic_bytes*(mem->block_size),
           (cache0->protocol == MESI) ? 1:0,(cache0->protocol == DRAGON) ? bus->num_of_busupd:0);
    printf("cache size |  block size |  associativity\n");
    printf("%s  %s  %s\n",cache_size,block_size,associativity)
    printf("core 1\n");
    printf("compute cycle |  load/store  | idls cycle  |  miss rate  |  access to private data  |  access to shared data\n");
    printf("%ld  %ld  %ld  %ld  %ld  %ld\n",pro0->compute_cycle,pro0->num_of_LS,pro0->idle_cycle,cache0->miss_time,cache0->access_pdata,cache0->access_sdata);
    printf("core 2\n");
    printf("compute cycle |  load/store  | idls cycle  |  miss rate  |  access to private data  |  access to shared data\n");
    printf("%ld  %ld  %ld  %ld  %ld  %ld\n",pro1->compute_cycle,pro1->num_of_LS,pro1->idle_cycle,cache1->miss_time,cache1->access_pdata,cache1->access_sdata);
    printf("core 3\n");
    printf("compute cycle |  load/store  | idls cycle  |  miss rate  |  access to private data  |  access to shared data\n");
    printf("%ld  %ld  %ld  %ld  %ld  %ld\n",pro2->compute_cycle,pro2->num_of_LS,pro2->idle_cycle,cache2->miss_time,cache2->access_pdata,cache2->access_sdata);
    printf("core 4\n");
    printf("compute cycle |  load/store  | idls cycle  |  miss rate  |  access to private data  |  access to shared data\n");
    printf("%ld  %ld  %ld  %ld  %ld  %ld\n",pro3->compute_cycle,pro3->num_of_LS,pro3->idle_cycle,cache3->miss_time,cache3->access_pdata,cache3->access_sdata);
    return 0;
}
