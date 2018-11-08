#ifndef DRAGON
#define DRAGON
struct msg* handle_msg_fromCPU_dragon(struct cache_block* block, struct msg* msg);

struct msg* handle_msg_fromBUS_dragon(struct cache_block* block, struct msg* msg);
#endif // DRAGON
