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

