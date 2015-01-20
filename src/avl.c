#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "avl.h"

struct node { 
	struct node* p;
	struct node* l; 
	struct node* r;
	int8_t bf;
	void* data;
}; 

struct avltree {
	int8_t (*greater_comparator)(void*,void*);
	node* root;
	uint32_t size;
};

avltree* avl_init_tree(int8_t (*g_c)(void*, void*)) {
	avltree* t = malloc(sizeof(*t));
	t->greater_comparator = g_c;
	t->root = (node*)0;
	t->size = 0;
	return t;
}

uint64_t delete(avltree* t, void* data);

node* first(avltree* t);
node* next(avltree* t, node* n);
node* prev(avltree* t, node* n);
node* get_node(avltree* t, void* data);

node* delete_node(avltree* t, node* n);
node* get_node_by_node(avltree* t, node* n, void* data);
void ff_insert(avltree* t, node* n, node* new);
void ff_rebalance_rest(avltree* t, node* n);

void rr_rotate(node*, node**);
void rl_rotate(node*, node**);
void lr_rotate(node*, node**);
void ll_rotate(node*, node**);

node* first(avltree* t) {
	node* n = t->root; 
	while(n && n->l)
		n = n-> l;
	return n;
}

void debug_print(avltree* t) {
	void node_print(int level, node* n) {
		if(n->r)
			node_print(level+1, n->r);
		int l;
		for(l = level; l > 0; l--)
			printf("\t");
		printf("[%d] %ld\n", n->bf, n->data);

		if(n->l)
			node_print(level+1, n->l);
	}
	if(t->root)
		node_print(0, t->root);
	printf("========\n");
}

node* next(__attribute__((unused)) avltree* t, node* n) {
	if(n->r) {
		n = n->r;
		while(n->l)
			n = n->l;
	} else {
		while(!(n->p) || n->p->l != n) {
			if(!(n = n->p))
				return 0;
		}
		n = n->p;
	}
	return n;
}

node* prev(__attribute__((unused)) avltree* t, node* n) {
	if(n->l) {
		n = n->l;
		while(n->r)
			n = n->r;
	} else {
		node * nn;
		do {
			nn = n;
			n = n->p;
		} while(n && n->r != nn);
	}
	return n;
}

uint64_t delete(avltree* t, void* data) {
	node* n = get_node(t, data);
	if(!n) {
		return -1;
	}
	delete_node(t, n);
	free(n);
	t->size--;
	return 0;
}
node* get_node(avltree* t, void* data) {
	return get_node_by_node(t, t->root, data);
}

node* get_node_by_node(avltree* t, node* n, void* data) {
	if(!n)
		return n;
	if(n->data == data) 
		return n;
	if(t->greater_comparator(data, n->data) > 0) {
		return get_node_by_node(t, n->l, data);
	} else {
		return get_node_by_node(t, n->r, data);
	}

}

node* delete_node(avltree* t, node* n) {
	printf("delete '%x'%d'\n", t, get_data(n));
	node* np = 0;
	uint8_t nested;
	if(n->l && n->r) {
		np = delete_node(t, prev(t, n));
		printf("-- %d\n", get_data(np)); /// REMOVEME
		nested = 0;
	} else {
		if(n->l) {
			np = n->l;
			n->l = 0;
		} else {
			np = n->r;
			n->r = 0;
		}
		n->bf = 0;
		nested = 1;
	}

	if(np) {
		np->r = n->r;
		np->l = n->l;
		if(np->l)
			np->l->p = np;
		np->p = n->p;
		if(np->r)
			np->r->p = np;
		np->bf = n->bf;
	}

	if(n->p) {
		if(n->p->l == n)
			n->p->l = np;
		else 
			n->p->r = np;
	} else {
		t->root = np;
	}

	if(nested) {
		node* ns;
		node* nr = np;
		np = n->p;
		while(np) {
			if(!np->bf) {
				if(np->l == nr)
					np->bf = -1;
				else 
					np->bf = 1;
				break;
			} else {
				//if(!((np->bf != 1 || np->l != nr) && (np->bf != -1 || np->r != nr))) {
				if(((np->bf == 1) && (np->l == nr)) || ((np->bf == -1) && (np->r == nr))) {
					np->bf = 0;
					nr = np;
					np = np->p;
				} else {
					ns = np->l == nr ? np->r : np->l;
					if(!ns->bf) {
						if(np->bf == 1)
							ll_rotate((np), &t->root);
						else 
							rr_rotate((np), &t->root);
						break;
					} else if(ns->bf == np->bf) {
						if(np->bf == 1)
							ll_rotate((np), &t->root);
						else 
							rr_rotate((np), &t->root);
						nr = ns;
						np = ns->p;
					} else {
						if(np->bf == 1)
							lr_rotate((np), &t->root);
						else
							rl_rotate((np), &t->root);
						nr = np->p;
						np = nr->p;
					}
				}
			}
		}
	}
	return n;
}

uint64_t get_node_num(avltree* r) {
	return r->size;
}

void* get_data(node* n) {
	return n->data;
}
	
uint64_t insert(avltree* t, void* data) {
	node* nn = malloc(sizeof(*nn));
	if(!nn)
		return -1;
	nn->data = data;
	nn->r = nn->l = nn->p = (node*) 0;
	nn->bf = 0;
	if(t->root)
		ff_insert(t, t->root, nn);
	else 
		t->root = nn;
	t->size++;
	return 0;
}

void ff_insert(avltree* t, node* n, node* new) {
	if(t->greater_comparator(n->data, new->data) <0) {
		//left
		if(n->l)
			ff_insert(t, n->l, new);
		else  {
			n->l = new;
			new->p = n;
			if(n->bf)
				n->bf = 0;
			else {
				n->bf = 1;
				ff_rebalance_rest(t, n);
			}
		}
	} else {
		//right
		if(n->r)
			ff_insert(t, n->r, new);
		else {
			n->r = new;
			new->p = n;
			if(n->bf)
				n->bf = 0;
			else {
				n->bf = -1;
				ff_rebalance_rest(t, n);
			}
		}
	}
}

void ff_rebalance_rest(avltree* t, node* n) {
	if(n->p && n->p->bf) {
		if(n->p->bf == 1) {
			if(n->p->r == n) {
				n->p->bf = 0;
			} else {
				debug_print(t);
				if(n->bf == -1) {
					lr_rotate((n->p), &(t->root));
				} else {
					ll_rotate((n->p), &(t->root));
				}
			}
		} else {
			if(n->p->l == n) {
				n->p->bf = 0;
			} else {
				debug_print(t);
				if(n->bf == 1)
					rl_rotate((n->p), &(t->root));
				else 
					rr_rotate((n->p), &(t->root));
			}
		}
	} else if(n->p) {
		n->p->bf = n->p->l == n ? 1 : -1;
		ff_rebalance_rest(t, n->p);
	}
}

void ll_rotate(node* origin, node** root) {
#ifdef _DEBUG_AVL
	printf("ll [%d]\n", (origin)->data);
#endif
	node* n = origin;
	node* ln = n->l;
	node* lrn = ln->r;

	if(*root == n) 
		*root = ln;
	else if(origin->p->l == origin)
		origin->p->l = ln;
	else 
		origin->p->r = ln;

	n->l = lrn;
	ln->r = n;

	ln->p = n->p;
	n->p = ln;
	if(lrn)
		lrn->p = n;

	if(ln->bf == 1)
		ln->bf = n->bf = 0;
	else 
		ln->bf = -1, n->bf = 1;
}

void rr_rotate(node* origin, node** root) {
#ifdef _DEBUG_AVL
	printf("rr [%d]\n", (origin)->data);
#endif
	node* n = origin;
	node* rn = n->r;
	node* rln = rn->l;

	if(*root == n)
		*root = rn;
	else if(origin->p->l == origin)
		origin->p->l = rn;
	else 
		origin->p->r = rn;

	n->r = rln;
	rn->l = n;

	rn->p = n->p;
	n->p = rn;
	if(rln)
		rln->p = n;

	if(rn->bf == -1)
		n->bf = rn->bf = 0;
	else 
		rn->bf = 1, n-> bf = -1;
}

void rl_rotate(node* origin, node** root) {
#ifdef _DEBUG_AVL
	printf("rl [%d]\n", (origin)->data);
#endif
	node* n = origin;
	//node* ln = n->l;
	node* rn = n->r;
	node* rln = rn->l;
	//node* rrn = rn->r;
	node* rlln = rln->l;
	node* rlrn = rln->r;

	if(*root == n)
		*root = rln;
	else if(origin->p->l == origin)
		origin->p->l = rln;
	else 
		origin->p->r = rln;

	rln->l = n;
	rln->r = rn;
	n->r = rlln;
	rn->l = rlrn;

	rln->p = n->p;
	n->p = rln;
	rn->p = rln;
	if(rlln)
		rlln->p = n;
	if(rlrn)
		rlrn->p = rn;

	if(rln->bf == -1)
		n->bf = 1;
	else 
		n->bf = 0;
	if(rln->bf == 1)
		rn->bf = -1;
	else 
		rn->bf = 0;
	rln->bf = 0;
}

void lr_rotate(node* origin, node** root) {
#ifdef _DEBUG_AVL
	printf("lr [%d]\n", (origin)->data);
#endif
	node* n = origin;
	node* ln = n->l;
	//node* rn = n->r;
	//node* lln = ln->l;
	node* lrn = ln->r;
	node* lrln = lrn->l;
	node* lrrn = lrn->r;

	if(*root == n)
		*root = lrn;
	else if(origin->p->l == origin)
		origin->p->l = lrn;
	else 
		origin->p->r = lrn;

	lrn->l = ln;
	lrn->r = n;
	ln->r = lrln;
	n->l = lrrn;

	lrn->p = n->p;
	ln->p = lrn;
	n->p = lrn;
	if(lrln)
		lrln->p = ln;
	if(lrrn)
		lrrn->p = n;

	if(lrn->bf == 1)
		n->bf = -1;
	else 
		n->bf = 0;
	if(lrn->bf == -1)
		ln->bf = 1;
	else 
		ln->bf = 0;
	lrn->bf = 0;
}

int8_t canonical_pointer_comparator(void* lhs, void* rhs) {
	return lhs > rhs ? 1 : -1 ;
}

#ifdef _DEBUG_AVL
main() {
#else
main_avl() {
#endif
	avltree* t = avl_init_tree(canonical_pointer_comparator);
	insert(t, 10);
	insert(t, 20);
	insert(t, 30);
	debug_print(t);
	printf("status %d\n", delete(t, 20));
	printf("removed\n");
	debug_print(t);
}
