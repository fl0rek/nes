#include <stdint.h>
#include <stdlib.h>

typedef struct record record;

struct record {
	size_t s;
	uint8_t dirty;
	uint16_t o_computername;
	uint16_t o_name;
	uint16_t o_surname;
	uint8_t type;
	uint16_t o_workspace;
	uint16_t o_last;
	uint8_t ip[4];
	char data[];
};

record* create_record(
		const uint8_t ip[4],
		const char* computername,
		const char* name, 
		const char* surname, 
		const uint8_t type,
		const char* workspace
		);

const char* magic_get(const record*, const uint16_t);
record* magic_set(record*, uint16_t*, const char*);
size_t get_size(const record*);

const uint8_t* get_ip(record* r);
void set_ip(record* r, const uint8_t ip[4]);

record* consolidate(record*);
