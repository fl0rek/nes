#include "marshall.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

typedef struct marshall_data marshall_data;

struct marshall_data {
	void* data;
	size_t s;
	marshall_data* next;
};

struct o_marshall {
	int fd;
	marshall_data* sdata;
	marshall_data* fdata;
	size_t overall_size;
};

struct i_marshall {
	void* data;
	size_t size;
	int fd;
};

o_marshall* m_init(const char* filename) {
	o_marshall* m = malloc(sizeof(*m));
	if(!(m->fd = open(filename, O_RDWR |O_CREAT |O_TRUNC, (mode_t)0600))) {
		free(m);
		return 0;
	}
	m->sdata = m->fdata = 0;
	m->overall_size = 0;

	return m;
}

uint32_t m_insert(o_marshall* m, void* d, size_t s) {
	marshall_data* md = malloc(sizeof(*md));
	if(!md)
		return -1;
	md->data = d;
	md->s = s;
	md->next = 0;
	if(m->sdata) {
		m->fdata->next = md;
		m->fdata = md;
	} else 
		m->sdata = m->fdata = md;
	m->overall_size += s;
	return 0;
}

uint32_t m_save(o_marshall* m) {
	marshall_data* md = m->sdata;
	ftruncate(m->fd, m->overall_size);
	void* dest = mmap(0, m->overall_size, PROT_WRITE, MAP_SHARED, m->fd, 0);
	if(!dest) {
		return -1;
	}
	void* dest_start = dest;
	((uint32_t*)dest)[0] = (uint32_t)1;
	while(md) {
		memcpy(dest, md->data, md->s);
		dest += md->s;
		md = md->next;
	}
	msync(dest_start, m->overall_size, MS_SYNC);
	munmap(dest_start, m->overall_size);
	
	return 0;
}

uint32_t m_close(o_marshall* m) {
	marshall_data* md;
	marshall_data* mdn;

	mdn = m->sdata;
	while(mdn) {
		md = mdn;
		mdn = mdn->next;
		free(md);
	}
	close(m->fd);
	free(m);
	return 0;
}

i_marshall* m_load(const char* filename) {
	i_marshall* m = malloc(sizeof(*m));
	struct stat fst;
	if(!(m->fd = open(filename, O_RDONLY, (mode_t)0600))) { 
		free(m);
		return 0;
	}
	fstat(m->fd, &fst);
	if(!(m->size = fst.st_size)) {
		close(m->fd);
		free(m);
		return 0;
	}
	
	if(!( m->data = mmap(0, m->size, PROT_READ, MAP_SHARED, m->fd, 0))) {
		close(m->fd);
		free(m);
		return 0;
	}
	return m;
}

void* m_get_data(i_marshall* m) {
	return m->data;
}

size_t m_get_size(i_marshall* m) {
	return m->size;
}

uint32_t m_load_close(i_marshall* m) {
	msync(m->data, m->size, MS_SYNC);
	munmap(m->data, m->size);
	close(m->fd);
	free(m);
}
