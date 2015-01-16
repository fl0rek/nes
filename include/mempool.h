#include <stddef.h> 
#include <stdint.h>

typedef struct mempool mempool;

mempool* init_empty_mempool(size_t o_size, uint64_t num);
mempool* init_from_array(void* arr, size_t o_size, uint64_t num);

uint64_t insert(mempool* m, void* data, uint32_t num);
uint64_t delete(mempool* m, void* data);
