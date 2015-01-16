#pragma once 
#include <inttypes.h>

struct node;
struct avltree;

typedef struct node node;
typedef struct avltree avltree;

avltree* init_avl_tree(int8_t (*g_c)(void*, void*));

uint64_t delete(avltree* t, void* data);
uint64_t insert(avltree* t, void* data);

uint64_t get_node_num(avltree* r);
void* get_data(node* n);

node* first(avltree* t);
node* next(avltree* t, node* n);
node* prev(avltree* t, node* n);

/*
void ff_insert(avltree* t, node* n, node* new);
void ff_rebalance_rest(avltree* t, node* n);

void rr_rotate(node*, node**);
void rl_rotate(node*, node**);
void lr_rotate(node*, node**);
void ll_rotate(node*, node**);
*/
