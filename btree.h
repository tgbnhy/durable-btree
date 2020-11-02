#ifndef BTREES_H
#define BTREES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#define MAX_KEY_LEN 15

static int btree_size = 4;  // number of pointers of each btree_node
/*
 * the key
 */
typedef union{
    int ivalue;
    double dvalue;
    char cvalue[4];
} key_type;
/*
 * Btree btree_node
 */
typedef struct btree_node {//Btree btree_node
    void **pointers;
    key_type **keys;
    struct btree_node *parent;
    int num_keys;
    bool is_leaf;
} btree_node;
/*
 * store the result of the range search
 */
typedef struct bvalue{
	void *value;
	struct bvalue *next;
}bvalue_t;
typedef bvalue_t blist_t;
/*
 * the rowid
 */
typedef struct btree_record {
    int value;
} btree_record;


typedef struct queue {//the queue
    int capacity;
    int front;
    int rear;
    int size;
    btree_node **items;
} queue;
/*
 * the free block list
 */
typedef struct add_btree_node{
	int offset;               //offset = -1 means that this is the last btree_node; offset >0  means that there are offset byte.
	struct add_btree_node *next;
}add_btree_node;
/*
 *
 */
typedef struct index_head {// the index head
	btree_node *root;     //the root of the index tree
	void *base;           //the base address of the mmap
	int keys_type;        //store the type of the key
	int node_count;       //the number of the keys
	long int offset;           //the offset of the index file
	add_btree_node *next; //point to the next free block
} index_head;
/*
 * the data for test
 */
typedef struct data_btree_node{
	int no;
	int age;
	char name[12];
	char tel_number[12];
}data_btree_node;
/**********************************************************/
//the interface for using
index_head *create_index(char *database_name , char *table_name,char *key_name, int key_type, int count,char *file_name);
void *load_index(char *file_name);
btree_node *btree_insert(btree_node *root, key_type *key, int value,index_head *infor);//insert
btree_node *btree_delete(btree_node *root, key_type *key,index_head *infor);//delete
int btree_update(btree_node *root , key_type *key, int value,index_head *infor);//update
blist_t *btree_search_range(btree_node *root , key_type *low , key_type *high , index_head *infor);//range search
blist_t *btree_search_matched(btree_node *root , key_type *key , index_head *infor);
//*************************

void scan_tree(btree_node *root,index_head *infor);
btree_node *find_leaf(btree_node *root, key_type *key,index_head *infor);
// Insertion
btree_record *make_new_btree_record(int value,index_head *infor);
btree_node *make_new_btree_node(index_head *infor);
btree_node *make_new_leaf(index_head *infor);
btree_node *make_new_tree(key_type *key, int value,index_head *infor);
btree_node *make_new_root(btree_node *left, btree_node *right, key_type *key,index_head *infor);
btree_node *insert_into_parent(btree_node *root, btree_node *left, btree_node *right, key_type *key,index_head *infor);
void insert_into_btree_node(btree_node *nd, btree_node *right, int index, key_type *key,index_head *infor);
btree_node *insert_into_btree_node_after_splitting(btree_node *root, btree_node *nd, btree_node *right, int index, key_type *key,index_head *infor);
btree_node *insert_into_leaf_after_splitting(btree_node *root, btree_node *leaf, int index, key_type *key, btree_record *rec,index_head *infor);
void insert_into_leaf(btree_node *leaf, int index, key_type *key, btree_record *rec,index_head *infor);
// Deletion
void destroy_btree_node(btree_node *nd,index_head *infor);
void destroy_tree(btree_node *root,index_head *infor);
void remove_entry(btree_node *nd, int index,index_head *infor);
btree_node *delete_entry(btree_node *root, btree_node *nd, int index,index_head *infor);
btree_node *adjust_root(btree_node *root,index_head *infor);
int get_btree_node_index(btree_node *nd,void *base);
btree_node *coalesce_btree_nodes(btree_node *root, btree_node *nd, btree_node *neighbor, int nd_index,index_head *infor);
void distribute_btree_nodes(btree_node *nd, btree_node *neighbor, int nd_index,index_head *infor);

#endif
