#pragma once

#include <inttypes.h>
#include <stdlib.h>

typedef struct o_marshall o_marshall;
typedef struct i_marshall i_marshall;

struct o_marshall;
struct i_marshall;


o_marshall* m_init(const char* filename);
uint32_t m_insert(o_marshall* m, void* d, size_t s);
uint32_t m_save(o_marshall* m);
uint32_t m_close(o_marshall* m);

i_marshall* m_load(const char* filename);
void* m_get_data(i_marshall* m);
size_t m_get_size(i_marshall* m);
uint32_t m_load_close(i_marshall* m);
