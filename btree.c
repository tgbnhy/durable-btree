#include "btree.h"
#include "mmap.h"
void *alloc_add( int length, index_head *infor) //allocate the free address
{
	void *add;
	add_btree_node *p = (add_btree_node *)((long int)(infor->next) + (long int)infor->base);
	add_btree_node *q = p->next , *tmp;
	while(q){
		if(q->next == NULL || q->offset >= length){//when the add_btree_node is the last or the length is proper;
			break;
		}
		p = p->next;
		q = q->next;
	}
	if(p ==  (add_btree_node *)((long int)infor->next + (long int)infor->base)){//first btree_node
		add =  infor->base + infor->offset;
		infor->offset += (long int)length;
		infor->next =  (add_btree_node *)infor->offset;
		p = (add_btree_node *)((long int)infor->next + (long int)infor->base);
		p->next = NULL;
		p->offset = -1;
	}
	else{//interspace
		add = (void *)q;
		if (q->offset - length >= sizeof(add_btree_node)) {
			tmp = q;
			q =  (add_btree_node *)((long int) q + (long int)length);
			q->next = tmp->next;
			q->offset -= length;
			p->next = q;
		}
		else{
			p->next = q->next;
		}
	}
	return add;
}
void free_add(void *ptr , int length, index_head *infor){//free the address
	add_btree_node *p = (add_btree_node *)((long int)infor->next + (long int)infor->base);
	add_btree_node *q = (add_btree_node *)ptr;
	q->next = NULL;
	q->offset = length;
	while(p){
		if(p <= q &&  (add_btree_node *)((long int)p->next + (long int)infor->base) >= q){
			if(p + p->offset == q){//merge the two into one
				p->offset += q->offset;
			}
			else{//insert into it
				q->next = p->next;
				p->next = q;
			}
			break;
		}
		else
			p = p->next;
	}
}

queue *init_queue(int capacity) //init the queue for the display
{
	queue *q;
	q = malloc(sizeof(queue));
	q->items = malloc(sizeof(btree_node *) * capacity);
	q->capacity = capacity;
	q->front = q->rear = 0;
	q->size = 0;
	return q;
}

void enqueue(queue *q, btree_node *nd) {
	if (q->size == q->capacity) {
		fprintf(stderr, "the queue is full !!\n");
		return;
	}
	q->items[q->rear] = nd;
	q->rear = (q->rear + 1) % q->capacity;
	q->size++;
}

btree_node *dequeue(queue *q) {
	btree_node *nd;
	if (q->size == 0) {
		fprintf(stderr, "the queue is empty !!\n");
		return NULL;
	}
	nd = q->items[q->front];
	q->front = (q->front + 1) % q->capacity;
	q->size--;
	return nd;
}

void initialize_keys(key_type *keys , key_type *key_input ,int keys_type){
	if(keys_type == 0)//the key type is interger
		keys->ivalue = key_input->ivalue;
	else if(keys_type == 1)//the key type is float
		keys->dvalue = key_input->dvalue;
	else if(keys_type == 2)//the key type is char
		strcpy(keys->cvalue, key_input->cvalue); /// strcpy(root->keys[0], key)
}

int str_cmp(key_type *a , key_type *b,int keys_type){
	if(keys_type == 0){
		return a->ivalue - b->ivalue;
	}
	else if(keys_type == 1){
		return a->dvalue - b->dvalue;
	}
	else {
		return strcmp(a->cvalue , b->cvalue);
	}
}
int get_key_len(key_type *key,int type){//get the length of the key
	if(type == 2)
		return strlen(key->cvalue)+1;
	else
		return 4;
}
int get_level(btree_node *root, btree_node *nd , void *base) {
	int level = 0;
	while (nd != root) {
		nd = (btree_node *)((long int)nd->parent + (long int)base); ///nd = nd->parent;
		level++;
	}
	return level;
}

void scan_tree(btree_node *root , index_head *infor) //use the quene to Level Traversal
{
	queue *q;
	btree_node *nd;
	void **nd_pointers;
	key_type **nd_keys;
	int level, new_level, i;
	long int base;
	if (root == NULL) {
		printf("Empty tree !\n");
		exit(0);
	}
	q = init_queue(infor->node_count);
	enqueue(q, root);
	level = 0;
	base = (long int)infor->base;
	while (q->size > 0) {
		nd = dequeue(q);
		nd_pointers = (void **)((long int)nd->pointers + (long int)base);
		nd_keys = (key_type **)((long int)nd->keys + (long int)base);
		new_level = get_level(root, nd, (void *)base);
		if (new_level > level) {
			//printf("\n");
			level = new_level;
		}
		for (i = 0; i < nd->num_keys; i++){
			key_type *temp =  (key_type *)((long int)nd_keys[i] + (long int)base);
		//	printf("%d ", temp->ivalue); ///nd->keys[i]
		}
	//	printf("| ");
		if (!nd->is_leaf) {
			for (i = 0; i <= nd->num_keys; i++){
				enqueue(q, (void *)((long int)nd_pointers[i] + (long int)base)); ///nd->pointers[i]
			}
		}
	}
//	printf("\n");
	free(q->items);
	free(q);
}
index_head *create_index(char *database_name , char *table_name,char *key_name, int key_type, int count,char *file_name){
	//create the index file;
	int file_len;
	index_head *infor;
	strcpy(file_name, "../");
	strcat(file_name,database_name);
	strcat(file_name, "/");
	strcat(file_name,table_name);
	strcat(file_name, ".");
	strcat(file_name, key_name);
	strcat(file_name, ".idx");
	file_len = count * 90;//
	int fd = open(file_name, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1){
		return 0;
	}
	lseek(fd, file_len - 1, SEEK_SET);
	write(fd, "1", 1);
	close(fd);
	infor = (index_head *)open_mmap(file_name);
	infor->base = (void *)infor;
	infor->keys_type = key_type;//0:int 1:double 2:char
	infor->root = NULL;
	infor->node_count = 0;
	infor->offset = sizeof(index_head);//this is the last btree_node;
	infor->next = (add_btree_node *)sizeof(index_head);//the first space which can malloc
	add_btree_node *p = (add_btree_node *)((long int)infor->offset+(long int)infor->base);
	p->offset = 0;
	p->next = NULL;
	return infor;
}
void *load_index(char *file_name){//load the index created before
	index_head *infor;
	infor = (index_head *)open_mmap(file_name);
	infor->base = (void *)infor;
	btree_node *root = (btree_node *)((long int)infor->root+(long int)infor->base);
	if(infor->node_count != 0)
		scan_tree(root , infor);
	return infor;
}
btree_node *find_leaf(btree_node *root, key_type *key ,index_head *infor) //find the proper location for the key
{
	btree_node *nd;
	key_type **nd_keys;
	int i, count=0;
	void **nd_pointers;
	if (root == NULL)
		return root;
	nd = root;
	while (!nd->is_leaf) {
		nd_keys = (key_type **)((long int)nd->keys + (long int)infor->base);
		nd_pointers = (void **)((long int)nd->pointers+(long int)infor->base);
		for (i = 0; i < nd->num_keys && str_cmp((key_type *)((long int)nd_keys[i]+(long int)infor->base) , key, infor->keys_type)<=0 ; i++,count++);
		nd = (btree_node *) ((long int)nd_pointers[i]+(long int)infor->base); ///(btree_node *)nd->pointers[i]
	}
	return nd;
}
btree_node *get_min_btree_node(btree_node *root , void *base){//get the minimum btree_node of the tree
	btree_node *nd = root;
	void **nd_pointers;
	while(!nd->is_leaf){
		nd_pointers = (void **)((long int)nd->pointers+(long int)base);
		nd = (btree_node *) ((long int)nd_pointers[0]+(long int)base); ///(btree_node *)nd->pointers[i]
	}
	return nd;
}
btree_node *get_max_btree_node(btree_node *root,void *base){//get the maxnimum btree_node of the tree
	btree_node *nd = root;
	void **nd_pointers;
	while(!nd->is_leaf){
		nd_pointers = (void **)((long int)nd->pointers+(long int)base);
		nd = (btree_node *) ((long int)nd_pointers[nd->num_keys]+(long int)base); ///(btree_node *)nd->pointers[i]
	}
	return nd;
}
void add_to_listend(blist_t *head , void *temp , int count){// add the temp to the end of the list
	blist_t *q = head;
	while(count--){
		q = q->next;
		if(q->value == temp){
			return;
		}
	}
	blist_t *new = malloc(sizeof(blist_t));
	new->value = temp;
	new->next = NULL;
	q->next = new;
	head->value++;
}
blist_t *btree_search_range(btree_node *root , key_type *low , key_type *high , index_head *infor){//range search
	blist_t *head = malloc(sizeof(blist_t));
	head->next = NULL;
	head->value = 0;
	btree_node *leaf, *p;
	key_type *temp;
	int i,count=0;
	long int base = (long int)infor->base;
	if(low)
		leaf = find_leaf(root, low, infor);
	else
		leaf = get_min_btree_node(root,(void *)base);
	void **leaf_pointers = (void **)((long int)leaf->pointers + base),**p_pointers;
	key_type **leaf_keys =  (key_type **)((long int)leaf->keys + base),**b;
	for (i = 0; i < leaf->num_keys ; i++)
	{
		temp = (key_type *)((long int)leaf_keys[i]+(long int)infor->base);
		if(low == NULL || str_cmp((key_type *)((long int)leaf_keys[i]+base) , low, infor->keys_type) >= 0){
			btree_record *rec = (btree_record *) ((long int)leaf_pointers[i] + base);
			while (high == NULL || str_cmp(temp, high,infor->keys_type) <= 0) {
				add_to_listend(head, rec, count++);
				if (i < leaf->num_keys-1){
					i++;
					temp = (key_type *)((long int) leaf_keys[i] + base);
					rec = (btree_record *) ((long int)leaf_pointers[i] + base);
				}
				else if(leaf_pointers[btree_size - 1] != NULL){
					leaf = (btree_node *)((long int) leaf_pointers[btree_size - 1] +  base); //the right btree_node
					leaf_keys = (key_type **)((long int) leaf->keys + base);
					leaf_pointers = (void **)((long int) leaf->pointers + base);
					temp = (key_type *)((long int) leaf_keys[0] +  base);
					rec = (btree_record *) ((long int)leaf_pointers[0] + base);
					i = 0;
				}
				if( leaf_pointers[btree_size - 1] == NULL && i == leaf->num_keys-1){
					add_to_listend(head, rec, count++);
					break;
				}
			}
			break;
		}
	}
	return head;
}

blist_t *btree_search_matched(btree_node *root , key_type *key , index_head *infor){
	blist_t *head = malloc(sizeof(blist_t));
	head->next = NULL;
	long int base = (long int)infor->base;
	btree_node *leaf;
	int i;
	leaf = find_leaf(root, key, infor);
	void **leaf_pointers = (void **)((long int)leaf->pointers + base);
	key_type **leaf_keys =  (key_type **)((long int)leaf->keys + base);
	if (leaf == NULL)
		return NULL;
	for (i = 0; i < leaf->num_keys && str_cmp((key_type *)((long int)leaf_keys[i]+base) , key, infor->keys_type) != 0; i++);
	if (i == leaf->num_keys)
		return NULL;
	btree_record *temp = (btree_record *) ((long int)leaf_pointers[i] + base); ///(btree_record *)leaf->pointers[i]
	if(temp)
		head->value = (void *)temp;
	else if(temp == NULL)
		head->value = NULL;
	return head;
}
btree_record *make_new_btree_record(int value , index_head *infor) {
	btree_record *rec;
	rec = (btree_record *) alloc_add(sizeof(btree_record),infor); ///(btree_record *)malloc(sizeof(btree_record))
	rec->value = value;
	return rec;
}

btree_node *make_new_btree_node(index_head *infor) {
	btree_node *nd;
	nd = (btree_node *) alloc_add(sizeof(btree_node),infor); ///(btree_node *)malloc(sizeof(btree_node))
	nd->pointers = (void **) (alloc_add(btree_size * sizeof(void *),infor) - infor->base); ///malloc(size * sizeof(void *))
	nd->keys = (key_type **) (alloc_add((btree_size - 1) * sizeof(key_type *),infor) - infor->base); ///malloc((size - 1) * sizeof(char *))
	nd->parent = NULL;
	nd->num_keys = 0;
	nd->is_leaf = false;
	return nd;
}

btree_node *make_new_leaf(index_head *infor) {
	btree_node *leaf;
	leaf = make_new_btree_node(infor);
	leaf->is_leaf = true;
	return leaf;
}

btree_node *make_new_tree(key_type *key, int value , index_head *infor) {
	btree_node *root;
	btree_record *rec;
	root = make_new_leaf(infor);
	int type_len = get_key_len(key,infor->keys_type);
	void **root_pointers = (void **)((long int)root->pointers +  (long int)infor->base);
	key_type **root_keys = (key_type **)((long int)root->keys + (long int) infor->base);
	rec = make_new_btree_record(value,infor);
	root_pointers[0] = (void *) ((long int)rec - (long int)infor->base); //store the offset of rec
	root_keys[0] = (key_type *) (alloc_add(type_len,infor) - (long int)infor->base); /// malloc(MAX_KEY_LEN)
	key_type *temp=(key_type *)((long int)root_keys[0] + (long int) infor->base);
	initialize_keys(temp , key , infor->keys_type); /// strcpy(root->keys[0], key)
	root_pointers[btree_size-1] = NULL; ///root->pointers[size-1]
	root->num_keys++;
	return root;
}

btree_node *make_new_root(btree_node *left, btree_node *right, key_type *key, index_head *infor) {
	btree_node *root;
	root = make_new_btree_node(infor);
	long int base =(long int)infor->base;
	int type_len = get_key_len(key,infor->keys_type);
	void **root_pointers = (void **)((long int)root->pointers + base);
	key_type **root_keys = (key_type **)((long int)root->keys + base);
	root_pointers[0] = (void *)((long int)left - base); /// root->pointers[0] = left;
	root_pointers[1] = (void *)((long int)right - base); ///root->pointers[1] = right;
	root_keys[0] = (key_type *) (alloc_add(type_len,infor) - base); /// malloc(MAX_KEY_LEN)
	key_type *temp=(key_type *)((long int)root_keys[0] + base);
	initialize_keys(temp , key, infor->keys_type);
	root->num_keys++;
	left->parent = (btree_node *) ((long int)root - base); ///root
	right->parent = (btree_node *) ((long int)root - base); ///root
	return root;
}

int btree_update(btree_node *root , key_type *key, int value,index_head *infor){
	blist_t *r;
	r = btree_search_matched(root , key ,infor);
	if(r){
		btree_record *temp = (btree_record *)r->value;
		temp->value = value;
		return 1;
	}
	return 0;
}
btree_node *btree_insert(btree_node *root, key_type *key, int value,index_head *infor) {
	btree_record *rec;
	btree_node *leaf;
	int index, cond;
	leaf = find_leaf(root, key,infor);
	if (!leaf) {  // cannot find the leaf, the tree is empty
		infor->node_count++;
		return make_new_tree(key, value,infor);
	}
	key_type **leaf_keys = (key_type **)((long int)leaf->keys+(long int)infor->base);
	key_type *temp = (key_type *)((long int)leaf_keys[0]+ (long int) infor->base);
	for (index = 0;index < leaf->num_keys && (cond = str_cmp((key_type *)((long int)leaf_keys[index]+ (long int) infor->base), key,infor->keys_type)) < 0; index++);//strcmp(leaf->keys[index], key)
	if (cond == 0) {  // ignore duplicates,here we can change to store the duplicates by using the linklist
		return root;
	}
	infor->node_count++;
	rec = make_new_btree_record(value,infor);
	if (leaf->num_keys < btree_size - 1) {
		insert_into_leaf(leaf, index, key, rec,infor);
		return root;  // the root remains unchanged
	}
	return insert_into_leaf_after_splitting(root, leaf, index, key, rec, infor);
}

btree_node *insert_into_parent(btree_node *root, btree_node *left, btree_node *right, key_type *key , index_head *infor) {
	btree_node *parent;
	int index, i;
	parent = (btree_node *)left->parent;
	if (parent == NULL) {
		return make_new_root(left, right, key, infor);
	}
	parent = (btree_node *)((long int)left->parent + (long int) infor->base);  ///parent = left->parent;
	key_type **parent_pointers = (key_type **)((long int)parent->pointers+(long int)infor->base);
	for (index = 0;index < parent->num_keys && parent_pointers[index] != (key_type *)((long int)left - (long int) infor->base); index++);  ///parent->pointers[index] != left
	if (parent->num_keys < btree_size - 1) {
		insert_into_btree_node(parent, right, index, key, infor);
		return root;  // the root remains unchanged
	}
	return insert_into_btree_node_after_splitting(root, parent, right, index, key, infor);
}

void insert_into_btree_node(btree_node *nd, btree_node *right, int index, key_type *key ,index_head *infor) {
	int i;
	void **nd_pointers = (void **) ((long int)nd->pointers + (long int) infor->base);
	key_type **nd_keys = (key_type **) ((long int)nd->keys + (long int) infor->base);
	int type_len = get_key_len(key,infor->keys_type);
	for (i = nd->num_keys; i > index; i--) {
		nd_keys[i] = nd_keys[i - 1];  ///nd->keys[i] = nd->keys[i-1]
		nd_pointers[i + 1] = nd_pointers[i]; ///nd->pointers[i+1] = nd->pointers[i];
	}
	nd_keys[index] = (key_type *) (alloc_add(type_len, infor) - (long int)infor->base); /// malloc(MAX_KEY_LEN)
	key_type *temp=(key_type *)((long int)nd_keys[index] + (long int) infor->base);
	initialize_keys(temp , key, infor->keys_type);
	nd_pointers[index + 1] = (void *)((long int)right - (long int) infor->base);  ///nd->pointers[index+1]
	nd->num_keys++;
}

btree_node *insert_into_btree_node_after_splitting(btree_node *root, btree_node *nd, btree_node *right, int index, key_type *key , index_head *infor) {
	int i, split;
	btree_node **temp_ps, *new_nd, *child;
	key_type **temp_ks, *new_key;
	void **nd_pointers = (void **) ((long int)nd->pointers + (long int) infor->base);
	key_type **nd_keys = (key_type **) ((long int)nd->keys + (long int) infor->base);
	int type_len = get_key_len(key,infor->keys_type);
	void **new_nd_p;
	key_type **new_nd_k;
	temp_ps = malloc((btree_size + 1) * sizeof(btree_node *));
	temp_ks = malloc(btree_size * sizeof(key_type *));
	for (i = 0; i < btree_size + 1; i++) {
		if (i == index + 1)
			temp_ps[i] = (btree_node *)((long int)right - (long int) infor->base);
		else if (i < index + 1)
			temp_ps[i] = nd_pointers[i];  ///nd->pointers[i]
		else
			temp_ps[i] = nd_pointers[i - 1];  ///nd->pointers[i-1]
	}
	for (i = 0; i < btree_size; i++) {
		if (i == index) {
			temp_ks[i] = (key_type *) (alloc_add(type_len,infor) - (long int)infor->base); /// malloc(MAX_KEY_LEN)
			key_type *temp=(key_type *)((long int)temp_ks[i] + (long int) infor->base);
			initialize_keys(temp,key,infor->keys_type);
		} else if (i < index)
			temp_ks[i] = nd_keys[i];
		else
			temp_ks[i] = nd_keys[i - 1];
	}
	split = btree_size % 2 ? btree_size / 2 + 1 : btree_size / 2;  // split is #pointers
	nd->num_keys = split - 1;
	for (i = 0; i < split - 1; i++) {
		nd_pointers[i] = temp_ps[i];
		nd_keys[i] = temp_ks[i];
	}
	nd_pointers[i] = temp_ps[i];  // i == split - 1
	new_key = (key_type *)((long int)temp_ks[split - 1]+(long int)infor->base);

	new_nd = make_new_btree_node(infor);
	new_nd_p = (void **) ((long int)new_nd->pointers + (long int) infor->base);
	new_nd_k = (key_type **) ((long int)new_nd->keys + (long int) infor->base);
	new_nd->num_keys = btree_size - split;
	for (++i; i < btree_size; i++) {
		new_nd_p[i - split] = temp_ps[i];
		new_nd_k[i - split] = temp_ks[i];
	}
	new_nd_p[i - split] = temp_ps[i];
	new_nd->parent = nd->parent;
	for (i = 0; i <= new_nd->num_keys; i++) {  //  #pointers == num_keys + 1
		child = (btree_node *) ((long int)new_nd_p[i]+(long int)infor->base);
		child->parent = (btree_node *)((long int)new_nd - (long int) infor->base);
	}
	free(temp_ps);
	free(temp_ks);
	return insert_into_parent(root, nd, new_nd, new_key,infor);
}

void insert_into_leaf(btree_node *leaf, int index, key_type *key, btree_record *rec,index_head *infor) // insert the btree_node to the leaf without spliting
{
	int i;
	void **leaf_pointers = (void **) ((long int)leaf->pointers + (long int) infor->base);
	key_type **leaf_keys = (key_type **) ((long int)leaf->keys + (long int) infor->base);
	int type_len = get_key_len(key,infor->keys_type);
	for (i = leaf->num_keys; i > index; i--) {
		leaf_keys[i] = leaf_keys[i - 1];
		leaf_pointers[i] = leaf_pointers[i - 1];
	}
	leaf_keys[index] = (key_type *) (alloc_add(type_len,infor) - (long int)infor->base); /// malloc(MAX_KEY_LEN)
	key_type *temp=(key_type *)((long int)leaf_keys[index] + (long int) infor->base);
	initialize_keys(temp,key,infor->keys_type);
	leaf_pointers[index] = (btree_record *)((long int)rec - (long int) infor->base);
	leaf->num_keys++;
}

btree_node *insert_into_leaf_after_splitting(btree_node *root, btree_node *leaf, int index, key_type *key, btree_record *rec,index_head *infor) {
	btree_node *new_leaf;
	btree_record **temp_ps;
	key_type **temp_ks, *new_key;
	int i, split;
	void **leaf_pointers = (void **) ((long int)leaf->pointers + (long int) infor->base);
	key_type **leaf_keys = (key_type **) ((long int)leaf->keys + (long int) infor->base);
	int type_len = get_key_len(key,infor->keys_type);
	void **new_leaf_pointers;
	key_type **new_leaf_keys;
	temp_ps = malloc(btree_size * sizeof(btree_record *));
	temp_ks = malloc(btree_size * sizeof(key_type *));
	for (i = 0; i < btree_size; i++) {
		if (i == index) {
			temp_ps[i] = rec;
			temp_ks[i] = (key_type *)alloc_add(type_len, infor);
			initialize_keys(temp_ks[i] ,key, infor->keys_type);
		} else if (i < index) {
			temp_ps[i] = (btree_record *)(leaf_pointers[i] + (long int) infor->base);
			temp_ks[i] = (key_type *)((long int)leaf_keys[i] + (long int) infor->base);
		} else {
			temp_ps[i] = (btree_record *)(leaf_pointers[i - 1] + (long int) infor->base);
			temp_ks[i] = (key_type *)((long int)leaf_keys[i - 1] + (long int) infor->base);
		}
	}
	split = btree_size / 2;
	leaf->num_keys = split;
	for (i = 0; i < split; i++) {
		leaf_pointers[i] = (void **)((long int)temp_ps[i] - (long int) infor->base);
		leaf_keys[i] = (key_type *)((long int)temp_ks[i] - (long int) infor->base);
	}
	new_leaf = make_new_leaf(infor);
	new_leaf->num_keys = btree_size - split;
	new_leaf_pointers = (void **) ((long int)new_leaf->pointers + (long int) infor->base);
	new_leaf_keys = (key_type **) ((long int)new_leaf->keys + (long int) infor->base);
	for (; i < btree_size; i++) {
		new_leaf_pointers[i - split] = (void **)((long int)temp_ps[i] - (long int) infor->base);
		new_leaf_keys[i - split] = (key_type *)((long int)temp_ks[i] - (long int) infor->base);
	}
	new_leaf->parent = leaf->parent;
	new_leaf_pointers[btree_size - 1] = leaf_pointers[btree_size - 1];
	leaf_pointers[btree_size - 1] = (void **)((long int)new_leaf - (long int) infor->base);
	free(temp_ps);
	free(temp_ks);
	new_key = (key_type *)((long int)new_leaf_keys[0] + (long int) infor->base);
	return insert_into_parent(root, leaf, new_leaf, new_key,infor);
}

void destroy_btree_node(btree_node *nd,index_head *infor) {
	free_add((key_type **) ((long int) nd->keys + (long int) infor->base) , btree_size * sizeof(key_type *),infor); ///free(nd->keys);
	free_add((void **) ((long int) nd->pointers + (long int) infor->base) , (btree_size - 1) * sizeof(char *),infor); ///free(nd->pointers);
	free_add(nd , sizeof(btree_node),infor);
}

void destroy_tree(btree_node *root,index_head *infor) {
	int i;
	if (!root->is_leaf) {
		for (i = 0; i < root->num_keys; i++) {
			free_add(*((char **) ((long int) root->keys + (long int) infor->base) + i) + (long int) infor->base , MAX_KEY_LEN,infor); /// free(root->keys[i]);
			destroy_tree(*((void **) ((long int) root->pointers + (long int) infor->base) + i) + (long int) infor->base,infor); ///destroy_tree(root->pointers[i])
		}
		destroy_tree(*((void **) ((long int) root->pointers + (long int) infor->base) + i) + (long int) infor->base,infor); /// destroy_tree(root->pointers[i]);
	} else {
		for (i = 0; i < root->num_keys; i++) {
			free_add(*((char **) ((long int) root->keys + (long int) infor->base) + i) + (long int) infor->base , MAX_KEY_LEN,infor); ///free(root->keys[i]);
			free_add(*((void **) ((long int) root->pointers + (long int) infor->base) + i) + (long int) infor->base , sizeof(btree_record),infor); // free btree_record free(root->pointers[i]);
		}
	}
	destroy_btree_node(root,infor);
}
btree_node *btree_delete(btree_node *root, key_type *key, index_head *infor) {
	btree_node *leaf;
	int i;
	leaf = find_leaf(root, key, infor);
	void **leaf_pointers = (void **) ((long int)leaf->pointers + (long int) infor->base);
	key_type **leaf_keys = (key_type **) ((long int)leaf->keys + (long int) infor->base);
	if (leaf == NULL)
		return root;
	for (i = 0;i < leaf->num_keys && str_cmp((key_type *)((long int)leaf_keys[i] + (long int) infor->base), key,infor->keys_type) != 0;i++);
	if (i == leaf->num_keys)  // no such key
	{
		printf("Sorry!Can't find!\n");
		return root;
	}
	root = delete_entry(root, leaf, i,infor);
	infor->root = (btree_node *)((long int)root - (long int)infor->base);
	infor->node_count--;
	return root;
}

btree_node *delete_entry(btree_node *root, btree_node *nd, int index,index_head *infor) {//delete the item in the nd
	int min_keys, cap, nd_index;
	btree_node *neighbor,*parent;
	void **nd_parent_pointers;
	remove_entry(nd, index,infor);
	if (nd == root)
		return adjust_root(nd,infor);//
	min_keys = nd->is_leaf ? btree_size / 2 : (btree_size - 1) / 2;
	if (nd->num_keys >= min_keys) {
		return root;
	}
	nd_index = get_btree_node_index(nd,infor->base);
	parent = (btree_node *)((long int)nd->parent + (long int) infor->base);
	nd_parent_pointers = (void **) ((long int)parent->pointers+ (long int) infor->base);
	if (nd_index == 0)
		neighbor = (btree_node *)((long int)nd_parent_pointers[1] + (long int) infor->base);  // right neighbor
	else
		neighbor = (btree_node *)((long int)nd_parent_pointers[nd_index - 1] + (long int) infor->base); // left neighbor
	cap = nd->is_leaf ? btree_size - 1 : btree_size - 2;//the capicity of the nd
	if (neighbor->num_keys + nd->num_keys <= cap)
		return coalesce_btree_nodes(root, nd, neighbor, nd_index,infor);
	distribute_btree_nodes(nd, neighbor, nd_index,infor);
	return root;
}

void distribute_btree_nodes(btree_node *nd, btree_node *neighbor, int nd_index ,index_head *infor) {
	int i;
	btree_node *tmp;
	void **nd_pointers = (void **) ((long int)nd->pointers + (long int) infor->base);
	key_type **nd_keys = (key_type **) ((long int)nd->keys + (long int) infor->base);
	btree_node *parent = (btree_node *)((long int)nd->parent + (long int) infor->base);
	void **nd_parent_pointers = (void **) ((long int)parent->pointers + (long int) infor->base);
	key_type **nd_parent_keys = (key_type **) ((long int)parent->keys + (long int) infor->base);
	void **neighbor_pointers = (void **) ((long int)neighbor->pointers + (long int) infor->base);
	key_type **neighbor_keys = (key_type **) ((long int)neighbor->keys + (long int) infor->base);
	if (nd_index != 0) {
		if (!nd->is_leaf)
			nd_pointers[nd->num_keys + 1] = nd_pointers[nd->num_keys];
		for (i = nd->num_keys; i > 0; i--) {  // shift to right by 1
			nd_keys[i] = nd_keys[i - 1];
			nd_pointers[i] = nd_pointers[i - 1];
		}
		if (!nd->is_leaf) {
			nd_keys[0] = nd_parent_keys[nd_index - 1];
			nd_pointers[0] = neighbor_pointers[neighbor->num_keys];
			tmp = (btree_node *) ((long int)nd_pointers[0] + (long int) infor->base);
			tmp->parent = (btree_node *)((long int)nd - (long int) infor->base);
			neighbor_pointers[neighbor->num_keys] = NULL;
			nd_parent_keys[nd_index - 1] = neighbor_keys[neighbor->num_keys - 1];
			neighbor_keys[neighbor->num_keys - 1] = NULL;
		} else {
			nd_keys[0] = neighbor_keys[neighbor->num_keys - 1];
			neighbor_keys[neighbor->num_keys - 1] = NULL;
			nd_pointers[0] = neighbor_pointers[neighbor->num_keys - 1];
			neighbor_pointers[neighbor->num_keys - 1] = NULL;
			initialize_keys((key_type *)((long int)nd_parent_keys[nd_index - 1] + (long int) infor->base) , (key_type *)((long int)nd_keys[0] + (long int) infor->base) , infor->keys_type);
			//strcpy((long int)nd_parent_keys[nd_index - 1] + (long int) infor->base, (long int)nd_keys[0] + (long int) infor->base);//  share the same key with child !!
		}
	} else {
		if (!nd->is_leaf) {
			nd_keys[nd->num_keys] = nd_parent_keys[0];  // link to father's key
			nd_pointers[nd->num_keys + 1] = neighbor_pointers[0];
			tmp = (btree_node *) ((long int)nd_pointers[nd->num_keys + 1] + (long int) infor->base);
			tmp->parent = (btree_node *)((long int)nd - (long int) infor->base);
			nd_parent_keys[0] = neighbor_keys[0];  //
		} else {
			nd_keys[nd->num_keys] = neighbor_keys[0];
			nd_pointers[nd->num_keys] = neighbor_pointers[0];
			initialize_keys((key_type *)((long int)nd_parent_keys[0] + (long int) infor->base) , (key_type *)((long int)neighbor_keys[1] + (long int) infor->base) , infor->keys_type);
			//strcpy(nd_parent_keys[0] + (long int) infor->base, neighbor_keys[1] + (long int) infor->base);// share the same key with chid !!
		}
		for (i = 0; i < neighbor->num_keys - 1; i++) {
			neighbor_keys[i] = neighbor_keys[i + 1];
			neighbor_pointers[i] = neighbor_pointers[i + 1];
		}
		neighbor_keys[i] = NULL;
		if (!nd->is_leaf)
			neighbor_pointers[i] = neighbor_pointers[i + 1];
		else
			neighbor_pointers[i] = NULL;
	}
	neighbor->num_keys--;
	nd->num_keys++;
}

btree_node *coalesce_btree_nodes(btree_node *root, btree_node *nd, btree_node *neighbor, int nd_index,index_head *infor) {//merge the btree_node
	int i, j, start, end;
	char *k_prime;
	btree_node *tmp, *parent;
	if (nd_index == 0) {  // make sure neighbor is on the left
		tmp = nd;
		nd = neighbor;
		neighbor = tmp;
		nd_index = 1;
	}
	parent = (btree_node *)((long int)nd->parent + (long int) infor->base);
	void **nd_pointers = (void **) ((long int)nd->pointers + (long int) infor->base);
	key_type **nd_keys = (key_type **) ((long int)nd->keys + (long int) infor->base);
	void **parent_pointers = (void **) ((long int)parent->pointers + (long int) infor->base);
	key_type **parent_keys = (key_type **) ((long int)parent->keys + (long int) infor->base);
	void **neighbor_pointers = (void **) ((long int)neighbor->pointers + (long int) infor->base);
	key_type **neighbor_keys = (key_type **) ((long int)neighbor->keys + (long int) infor->base);
	start = neighbor->num_keys;
	if (nd->is_leaf) {
		for (i = start, j = 0; j < nd->num_keys; i++, j++) {
			neighbor_keys[i] = nd_keys[j];
			neighbor_pointers[i] = nd_pointers[j];
			nd_keys[j] = NULL;
			nd_pointers[j] = NULL;
		}
		neighbor->num_keys += nd->num_keys;
		neighbor_pointers[btree_size - 1] = nd_pointers[btree_size - 1];
	} else {
		neighbor_keys[start] = alloc_add(MAX_KEY_LEN,infor) - (long int) infor->base;
		initialize_keys((key_type *)((long int)neighbor_keys[start] + (long int) infor->base) , (key_type *)((long int)parent_keys[nd_index - 1] + (long int) infor->base) , infor->keys_type);
		for (i = start + 1, j = 0; j < nd->num_keys; i++, j++) {
			neighbor_keys[i] = nd_keys[j];
			neighbor_pointers[i] = nd_pointers[j];
		}
		neighbor_pointers[i] = nd_pointers[j];
		neighbor->num_keys += nd->num_keys + 1;
		for (i = 0; i <= neighbor->num_keys; i++) {
			tmp = (btree_node *) ((long int)neighbor_pointers[i] + (long int) infor->base);
			tmp->parent = (btree_node *)((long int)neighbor - (long int) infor->base);
		}
	}
	destroy_btree_node(nd, infor);
	return delete_entry(root, parent, nd_index, infor);
}


int get_btree_node_index(btree_node *nd , void *base) {//get the it's rank in the parent
	btree_node *parent;
	int i;
	parent = (btree_node *)((long int)nd->parent + (long int) base);
	void **parent_pointers = (void **) ((long int)parent->pointers + (long int) base);
	for (i = 0; i < parent->num_keys && parent_pointers[i] != (void **)((long int)nd - (long int) base); i++);
	return i;
}

btree_node *adjust_root(btree_node *root,index_head *infor) {
	btree_node *new_root, **tmp;
	if (root->num_keys > 0)  // at least two childs
		return root;
	if (!root->is_leaf) {  // root has only one child
		tmp = (btree_node **)((long int)root->pointers + (long int)infor->base);
		new_root = (btree_node *)((long int)*tmp + (long int)infor->base);
		new_root->parent = NULL;
	} else
		new_root = NULL;
	destroy_btree_node(root,infor);
	return new_root;
}

void remove_entry(btree_node *nd, int index, index_head *infor) {
	int i, index_k;
	long int base =(long int)infor->base;
	void **nd_pointers = (void **) ((long int)nd->pointers + base);
	char **nd_keys = (char **) ((long int)nd->keys +base);
	if (nd->is_leaf) {
		for (i = index; i < nd->num_keys - 1; i++) {
			nd_keys[i] = nd_keys[i + 1];
			nd_pointers[i] = nd_pointers[i + 1];
		}
		nd_keys[i] = NULL;
		nd_pointers[i] = NULL;
	//	free_add((key_type *)((long int)nd_keys[i] + base) , sizeof(key_type) , infor);
	//	free_add((btree_record *)((long int)nd_pointers[i] + base) , sizeof(key_type), infor);  // destroy the btree_record
	} else {
		index_k = index - 1;  // index_p == index
		for (i = index_k; i < nd->num_keys - 1; i++) {
			nd_keys[i] = nd_keys[i + 1];
			nd_pointers[i + 1] = nd_pointers[i + 2];
		}
		nd_keys[i] = NULL;
		nd_pointers[i + 1] = NULL;
	//	free_add((char *)((long int)nd_keys[index_k] + (long int) infor->base),sizeof(btree_record),infor);
	}
	nd->num_keys--;
}
