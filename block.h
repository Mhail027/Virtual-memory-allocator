// Copyright Necula Mihail 313CAa 2023-2024
#ifndef BLOCK_H
#define BLOCK_H

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

#define u_int unsigned int

typedef struct block_t {
	void *start_adr;
	char *value;
	u_int nr_bytes;
} block_t;

int cmp_func_adr_of_blocks(void *a, void *b);
int cmp_func_size_of_blocks(void *a, void *b);

u_int count_bytes(adll_t *v);
u_int count_blocks(adll_t *v);

u_int convert_pointer_to_int(void *x);
void *find_min_adr(adll_t *v);

u_int block_is_in_vector(adll_t *v, void *start_adr, u_int nr_bytes);
adll_t *get_blocks_from_zone(adll_t *v, void *start_zone, void *stop_zone);
void remove_block_from_array(adll_t *v, void *start_adr);
void add_neighbours_to_block(adll_t *v, block_t *b_info);

#endif
