#include "mempool.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct mempool {
	size_t o_size;
	uint64_t m_size;
	uint64_t o_last;
	uint8_t* m_map; // true <=> empty
	uint64_t inconsistent;
	size_t aux_size;
	void* aux;
	void* pool;
};	

mempool* init_empty_mempool(size_t o_size, uint64_t num);
mempool* init_from_array(void* arr, size_t o_size, uint64_t num);

void set_aux(void* a, size_t s);
void* get_aux();

void cleanup_caller(mempool* m);
void cleanup_callee(mempool* m);

uint64_t insert(mempool* m, void* data, uint32_t num);
uint64_t delete(mempool* m, void* data);

uint64_t first(mempool* m);
uint64_t next(mempool* m, uint64_t e);
uint64_t prev(mempool* m, uint64_t e);

uint64_t first(mempool* m) {
	return next(m, 0);
}

uint64_t next(mempool* m, uint64_t e) {
	uint64_t id = ++e;
	for(; m->m_map[id] && id < m->o_last; id++);
	if(id >= m->o_last)
		return 0;
	return id;
}

uint64_t prev(mempool* m, uint64_t e) {
	uint64_t id = --e;
	for(; m->m_map[id] && id; id--);
	return id;
}


void* get(mempool* m, uint64_t id);

uint64_t resize_pool(mempool* m, uint64_t n_size);

uint64_t insert(mempool* m, void* data, uint32_t num) {
	uint64_t id;
	if(!m->inconsistent) {
		if(m->o_last == m->m_size) 
			if(resize_pool(m, m->m_size))
				return 0;
		id = m->o_last++;
	} else {
		for(id = 0; ! m->m_map[id] ; id++);
		m->m_map[id] = 0;
		m->inconsistent--;
	}

	void* ptr = m->pool + id * m->o_size;
	memcpy(ptr, data, m->o_size);
	return id;
}	

uint64_t delete(mempool* m, void* data) {
	uint64_t id = (data - m->pool) / m->o_size;
	m->m_map[id] = 1;
	return 0;
}

/**
 * invalidates ids
 * horray
 */

void consolidate(mempool* m) {
	uint64_t fid, bid;
	fid = 1; 
	bid = m->o_last;
	while(fid < bid) {
		if(m->m_map[fid] && m->m_map[bid])
			bid--;
		else if(m->m_map[fid] && !m->m_map[bid]) {
			memcpy(
					m->pool + fid *m->o_size,
					m->pool + bid *m->o_size,
					o_size
				);
			m->m_map[fid] = 0;
			m->m_map[bid] = 1;
			m->o_last--;
		} else if(!m->m_map[fid])
			fid++;
	}

}

int64_t serialize(mempool* m, FILE* fh) {
	consolidate(m);
	void* dest = mmap(0, m->o_last*m->o_size, PROT_WRITE, MAP_SHARED, fh, 0);
	memcpy(dest, m->pool, m->o_last*m->o_size);
	msync(dest, m->o_last *m->o_size, MS_SYNC);
	munmap(dest, m->o_last *m->o_size); 
}

uint64_t resize_pool(mempool* m, uint64_t n_size) {
	void* spawning_pool = realloc(m->pool +m->o_size, n_size);
	uint8_t* spawning_map = calloc(n_size, sizeof(*spawning_pool)); // calloc maybe?

	if(!spawning_pool || !spawning_map)
		return -1;

	m->pool = spawning_pool -m->o_size;
	m->m_map = spawning_map;
	m->m_size = n_size;
	return 0;
}

void* get(mempool* m, uint64_t id) {
       if(m->m_map[id])
	       return 0;	       
       return m->pool + (id *m->o_size);
}

mempool* init_from_array(void* arr, size_t o_size, uint64_t num) {
	mempool* m = malloc(sizeof(*m));
	if(!m || !arr)
		return 0;
	m->o_size = o_size;
	m->o_last = num;
	m->m_size = num;
	m->m_map = arr;
	m->inconsistent = 0;
	m->pool = calloc(m->m_size, m->o_size) - m->o_size;
	if(!m->m_map || !m->pool)
		return 0;
	return m;
}

mempool* init_empty_mempool(size_t o_size, uint64_t num) {
	void* p = malloc(num*o_size);
	mempool* m =  init_from_array(p, o_size, num);
	memset(m->m_map, 1, m->m_size);
	m->o_last = 1;
	return m;
}

/**
 * no return status because seriously what could we do? try calling it again at best
 * no m == 0 check, beacuese we want to fail fast.
 */

void cleanup_caller(mempool* m) {
	free(m->m_map);
	free(m);
}

void cleanup_calee(mempool* m) {
	free(m->pool);
	cleanup_caller(m);
}

int main() {
	mempool* m = init_empty_mempool(sizeof(int), 3);
	if(!m)
		printf("E_ALLOC");
	int* i = malloc(sizeof(int));
	*i = 10;
	uint64_t id1 = insert(m,(void*) i, 1);
	*i = 20;
	id1 = insert(m,(void*) i, 1);
	*i = 30;
	id1 = insert(m,(void*) i, 1);
	*i = 40;
	uint64_t id3 = insert(m,(void*) i, 1);

	uint64_t* it = (uint64_t*) first(m);
	printf(":%d\n", m->o_last);
	if(it)
		do {
			printf("- %d : ", it);
			printf("%d\n", *(int*)get(m, it));
		} while((it = next(m, it)));

	printf("%d\n", m->m_size);

	return 0;
}
