#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "btree.h"
//calculate the time
struct timeval starttime,endtime;
double timeuse;
/*
 * 插入时注意参数类型，0：整型，1：浮点型，2：字符串
 *
 */
btree_node *test_insert(btree_node *root , index_head *infor) {
	key_type *key;
	char *temp;
	int i, value;
	data_btree_node *data;
	data = (data_btree_node *)open_mmap("data.dat");
	key = malloc(MAX_KEY_LEN);
	temp = malloc(MAX_KEY_LEN);
	gettimeofday(&starttime,0);
	for (i = 0; i < 100; i++) {
		value = i;
		key->ivalue = data->no;//初始化key，整型
//		printf("aaa\n");
//		printf("%d\n",data->no);
		/************
		 *此处为调用插入接口，
		 */
		root = btree_insert(root, key, value , infor);
		infor->root = (btree_node *)((long int)root - (long int)infor->base);
//************************
		data++;
	}
	gettimeofday(&endtime,0);
	timeuse = 1000000*(endtime.tv_sec - starttime.tv_sec) + endtime.tv_usec - starttime.tv_usec;
	timeuse /=1000000*60;
	printf("it costs %fmin to create the index\n",timeuse);
	free(key);
	return root;
}
void test_find(btree_node *root , index_head *infor) {//单一匹配测试
	key_type *key;
	int temp;
	struct timeval starttime, endtime;
	data_btree_node *data = (data_btree_node*) open_mmap("data.dat"), *p;
	blist_t *r;
	key = malloc(MAX_KEY_LEN);
	printf("please input the key you want to find:");
	if (infor->keys_type == 0) {
		scanf("%d", &key->ivalue);
	} else if (infor->keys_type == 1)
		scanf("%lf", &(key->dvalue));
	else
		scanf("%s", key->cvalue);
	gettimeofday(&starttime, 0);
//**************
	r = btree_search_matched(root, key, infor);
//**************
	gettimeofday(&endtime, 0);
	timeuse = 1000000 * (endtime.tv_sec - starttime.tv_sec) + endtime.tv_usec
			- starttime.tv_usec;
	if (r) {
		btree_record *temp = (btree_record *) r->value;
		printf("btree_record of %d: %d\n", key->ivalue, temp->value);
		p = data;
		p += temp->value;
		printf("-------------------------------------------\n");
		printf("%d %d %11s %11s\n", p->no, p->age, p->name, p->tel_number);
		printf("-------------------------------------------\n");
	} else
		printf("Not found!!\n");
	printf("it costs %fμs to find the btree_record\n", timeuse);
	free(r);
	free(key);
}
void test_range_find(btree_node *root,index_head *infor) {//范围查询测试
	key_type *key, *key1;
	int temp, temp1;
	struct timeval starttime, endtime;
	data_btree_node *data = (data_btree_node*)open_mmap("data.dat") , *p;
	blist_t *r;
	key = malloc(MAX_KEY_LEN);
	key1 = malloc(MAX_KEY_LEN);
	printf("please input the low and high:");
	if (infor->keys_type == 0) {
		scanf("%d", &key->ivalue);
		scanf("%d", &key1->ivalue);
	} else if (infor->keys_type == 1)
		scanf("%lf", &(key->dvalue));
	else
		scanf("%s", key->cvalue);
	gettimeofday(&starttime, 0);
	r = btree_search_range(root, key, key1, infor);
	gettimeofday(&endtime, 0);
	timeuse = 1000000 * (endtime.tv_sec - starttime.tv_sec) + endtime.tv_usec
			- starttime.tv_usec;
	printf("the count is %d\n", (long int)(r->value));
	r = r->next;
	printf("-------------------------------------------\n");
	if (r) {
		while (r) {
			btree_record *temp = (btree_record *) r->value;
			p = data;
			p += temp->value;
			printf("%d %d %11s %11s\n", p->no, p->age, p->name, p->tel_number);
			r = r->next;
		}
	} else
		printf("Not found!!\n");
	printf("-------------------------------------------\n");
	printf("it costs %fμs to find the btree_record\n", timeuse);
	free(key);
	free(r);
}
btree_node *test_delete(btree_node *root,index_head *infor) {
	int temp;
	btree_node *tmp = root;
	key_type *key;
	printf("please input the key you want to delete:");
	scanf("%d", &temp);
	key =  malloc(MAX_KEY_LEN);
	key->ivalue = temp;
//*********
	root = btree_delete(root, key,infor);
	infor->root = (btree_node *)((long int)root - (long int)infor->base);
//**********
	free(key);
	return root;
}
void test_update(btree_node *root,index_head *infor){
	key_type *key;
	int value;
	key = malloc(MAX_KEY_LEN);
	printf("please input the key you want to update!\n");
	scanf("%d%d", &key->ivalue, &value);
	if(btree_update(root, key, value, infor))
		printf("Update successfully!\n");
	else
		printf("Update failed!\n");
	free(key);
}
int main()
{
	btree_node *root = NULL;
	index_head *infor;
	char file_name[50];
	/*
	 *创建索引，参数分别为：表名，键名，类型，记录数，文件名（需申请空间）
	 */
	if((infor = create_index("school","student", "no", 0, 100,file_name))){
		root = test_insert(root,infor);//插入
	//	test_find(root,infor);
	//	test_range_find(root,infor);
		root = test_delete(root,infor);
		test_update(root,infor);
		//printf("the offset is %d\n",infor->offset);
		close_mmap(infor->base , file_name);//解除映射
	}
	/*
	 * 根据已有索引文件导入索引，所有信息在infor里，调用
	 */
	infor = load_index(file_name);
	root = (btree_node *)((long int)infor->root+(long int)infor->base);
	test_find(root,infor);
	root = test_delete(root,infor);
	close_mmap(infor->base , file_name);
    return 0;
}
