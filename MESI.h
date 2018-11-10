#ifndef _MESI_
#define _MESI_
#include"Cache.h"

void handle_msg_fromCPU_MESI(struct cache_block* block, struct msg* msg,struct L1_cache *cache);

void handle_msg_fromBUS_MESI(struct cache_block* block, struct msg* msg,struct L1_cache *cache,long int cycle);
#endif // MESI
