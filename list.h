// Copyright Necula Mihail 313CAa 2023-2024
#ifndef LIST_H
#define LIST_H

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define u_int unsigned int

// dll_node = node from a double linked list
typedef struct dll_node_t {
	void *data;
	struct dll_node_t *prev, *next;
} dll_node_t;

// dll = doubly linked list
typedef struct dll_t {
	struct dll_node_t *head;
	u_int data_size;
	u_int size;
	int (*compare_function)(void *a, void *b);
} dll_t;

// addl = vector of doubly linked lists
typedef struct adll_t {
	struct dll_t **list;
	u_int size;
	u_int max_size;
	int (*compare_function)(void *a, void *b);
} adll_t;

void free_dll_node(dll_node_t *node);

dll_t *dll_create(u_int data_size, int (*compare_function)(void *, void *));
dll_node_t *dll_get_nth_node(dll_t *list, u_int n);
dll_node_t *dll_remove_nth_node(dll_t *list, u_int n);
void dll_add_nth_node(dll_t *list, u_int n, void *data);
void dll_add_node_in_order(dll_t *list, void *data);
void dll_free(dll_t *list);

adll_t *adll_create(u_int nr_lists, int (*compare_function)(void *, void *));
void addl_realloc(adll_t *v, u_int new_size);
void adll_add_nth_list(adll_t *v, u_int n, dll_t *list);
void adll_remove_nth_list(adll_t *free_dll, u_int n);
void adll_add_nd_in_ord(adll_t *v, void *d, u_int s, int (*c)(void *, void *));
void adll_free(adll_t *v);
void adll_unify(adll_t *dst, adll_t *src);

#endif
