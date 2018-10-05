#include "benchmark_list.h"
#include <stdio.h>
#include <stdint.h>
#include <numa.h>

#define MAXVAL 4056
typedef struct node {
	char key;
	struct node *child[2];
	char value[MAXVAL];
} node_t;

typedef struct tree {
	node_t *root;
} tree_t;

tree_t *tree;

static node_t *new_tree_node(int key,char* value,int value_len)
{
	node_t *node = malloc(sizeof(node_t));

 	node->key = key;
	node->child[0] = NULL;
	node->child[1] = NULL;

	int len = value_len;
	if(len>MAXVAL) len=MAXVAL;
	memcpy(node->value,value,value_len);

	return node;
}

pthread_data_t *alloc_pthread_data(void)
{
	pthread_data_t *d;
	size_t pthread_size;

	pthread_size = sizeof(pthread_data_t);
	pthread_size = CACHE_ALIGN_SIZE(pthread_size);

	d = (pthread_data_t *)malloc(pthread_size);
	return d;
}

void free_pthread_data(pthread_data_t *d)
{
}

void *list_global_init(int init_size, int key_range)
{
	node_t *prev, *cur, *new_node;
	int i, key, val, direction;

	tree = (tree_t *)malloc(sizeof(tree_t));

	if (tree == NULL){
		return NULL;
	}
	tree->root = new_tree_node(INT_MAX,"",0);

	i = 0;
	while (i < init_size) {
		key = rand() % key_range;

		prev = tree->root;
		cur = prev->child[0];
		direction = 0;
		while (cur != NULL) {
			prev = cur;
			val = cur->key;
			if (val > key) {
				direction = 0;
				cur = cur->child[0];
			} else if (val < key) {
				direction = 1;
				cur = cur->child[1];
			} else
				break;
		}
		if (cur != NULL)
			continue;
		new_node = new_tree_node(key,"",0);
		if (new_node == NULL){
			return NULL;
		}
		prev->child[direction] = new_node;
		i++;
	}

	return tree;
}

int list_thread_init(pthread_data_t *data, pthread_data_t **sync_data, int nr_threads)
{
	return 0;
}

void list_global_exit(void *list)
{
	//free l->head;
}

int list_ins(int key, char* value, int val_length, pthread_data_t *data)
{
	node_t *prev, *cur, *new_node;
	int direction, curkey;
	uint64_t ret;

	prev = (node_t *)(tree->root);
	cur = (node_t *)(prev->child[0]);
	direction = 0;
	while (cur != NULL) {
		curkey = cur->key;
		if (curkey > key) {
			direction = 0;
			prev = cur;
			cur = (node_t *)(cur->child[0]);
		} else if (curkey < key) {
			direction = 1;
			prev = cur;
			cur = (node_t *)(cur->child[1]);
		} else
			break;
	}
	ret = (cur == NULL);
	if (ret) {
		new_node = new_tree_node(key,value,val_length);
		prev->child[direction] = new_node;
	}

	return ret;
}


int list_del(int key, pthread_data_t *data)
{
	node_t *prev, *cur, *prev_succ, *succ, *next;
	node_t *cur_child_l, *cur_child_r;
	int direction, val;
	uint64_t ret;

	prev = (node_t *)(tree->root);
	cur = (node_t *)(prev->child[0]);
	direction = 0;
	while (cur != NULL) {
		val = cur->key;
		if (val > key) {
			direction = 0;
			prev = cur;
			cur = (node_t *)(cur->child[0]);
		} else if (val < key) {
			direction = 1;
			prev = cur;
			cur = (node_t *)(cur->child[1]);
		} else
			break;
	}

	ret = (cur != NULL);
	if (!ret)
		goto out;

	cur_child_l = (node_t *)(cur->child[0]);
	cur_child_r = (node_t *)(cur->child[1]);
	if (cur_child_l == NULL) {
		prev->child[direction] = cur_child_r;
		goto out;
	}
	if (cur_child_r == NULL) {
		prev->child[direction] = cur_child_l;
		goto out;
	}
	prev_succ = cur;
	succ = cur_child_r;
	next = (node_t *)(succ->child[0]);
	while (next != NULL) {
		prev_succ = succ;
		succ = next;
		next = (node_t *)(next->child[0]);
	}

	if (prev_succ == cur) {
		prev->child[direction] = succ;
		succ->child[0] = cur_child_l;
	} else {
		prev->child[direction] = succ;
		prev_succ->child[0] = (succ->child[1]);
		succ->child[0] = cur_child_l;
		succ->child[1] = cur_child_r;
	}

out:
	free(cur);
	return ret;
}

char* list_find(int key, pthread_data_t *data)
{
	node_t *cur;

	int val;
	uint64_t ret;

	cur = (node_t *)(tree->root);
	cur = (node_t *)(cur->child[0]);

	while (cur != NULL) {
		val = cur->key;
		if (val > key) {
			cur = (node_t *)(cur->child[0]);
		} else if (val < key) {
			cur = (node_t *)(cur->child[1]);
		} else
			break;
	}
	ret = (cur != NULL);
	if(cur == NULL) return NULL;
	else return cur->value;
}
