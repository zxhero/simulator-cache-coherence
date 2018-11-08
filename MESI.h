#ifndef MESI
#define MESI
struct msg* handle_msg_fromCPU_MESI(struct cache_block* block, struct msg* msg);

struct msg* handle_msg_fromBUS_MESI(struct cache_block* block, struct msg* msg);
#endif // MESI
