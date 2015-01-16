#include "backend.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*
struct record {
	size_t s;
	uint8_t dirty;
	uint16_t o_name;
	uint16_t o_workspace;
	uint16_t o_last;
	uint8_t ip[4];
	char data[];
};
*/

record* create_record(
		const uint8_t ip[4],
		const char* computername,
		const char* name, 
		const char* surname, 
		const uint8_t type,
		const char* workspace
		) {
	uint32_t computername_len = strlen(computername) +1;
	uint32_t name_len = strlen(name) +1;
	uint32_t surname_len = strlen(surname) +1;
	uint32_t workspace_len = strlen(workspace) +1;

	uint32_t data_len = computername_len + name_len + surname_len * workspace_len;
	size_t data_size = data_len *sizeof(*name);

	record* r;
	size_t overall_size = data_size + sizeof(*r);
	r = malloc(overall_size);

	r->s = overall_size;
	r->dirty = 0;
	r->ip[0] = ip[0];
	r->ip[1] = ip[1];
	r->ip[2] = ip[2];
	r->ip[3] = ip[3];
	r->type = type;

	uint32_t data_off = 0;

	strcpy(&r->data[data_off], computername);
	r->o_computername = data_off;
	data_off += computername_len;

	strcpy(&r->data[data_off], name);
	r->o_name = data_off;
	data_off += name_len;

	strcpy(&r->data[data_off], surname);
	r->o_surname = data_off;
	data_off += surname_len;

	strcpy(&r->data[data_off], workspace);
	r->o_workspace = data_off;
	data_off += workspace_len;

	r->o_last = data_off;
	return r;
}

const uint8_t* get_ip(record* r) {
	return r->ip;
}

void set_ip(record* r, const uint8_t ip[4]) {
	r->ip[0] = ip[0];
	r->ip[1] = ip[1];
	r->ip[2] = ip[2];
	r->ip[3] = ip[3];
}
	
record* consolidate(record* r) {
	if(!r->dirty)
		return r;
	record* nr = create_record(
			r->ip,
			magic_get(r, r->o_computername),
			magic_get(r, r->o_name),
			magic_get(r, r->o_surname),
			r->type,
			magic_get(r, r->o_workspace)
		);
	free(r);
	return nr;
}

const char* magic_get(const record* r, const uint16_t off) {
	return &r->data[off];
}

record* magic_set(record* r, uint16_t* off_ptr, const char* data) {
	uint16_t off = *off_ptr;

	record* nr = r;
	uint32_t new_data_len = strlen(data) +1;
	uint32_t old_data_len = strlen(&r->data[off]) +1;

	if(new_data_len <= old_data_len) {
		strcpy(&r->data[off] , data);
		if(!(new_data_len -old_data_len)) 
			r->dirty = 1;
	} else {
		nr = realloc(r, r->s + (new_data_len)+sizeof(*data));
		strcpy(&nr->data[nr->o_last], data);
		*off_ptr = nr->o_last;
		nr->o_last += new_data_len;
		nr->s += new_data_len +sizeof(*data);
		nr->dirty = 1;
	}
	return nr;
}

size_t get_size(const record* r) {
	return r->s;
}

