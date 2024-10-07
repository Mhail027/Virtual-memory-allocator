// Copyright Necula Mihail 313CAa 2023-2024
#include "block.h"
#include "list.h"

// Compare the start address of blocks a and b.
// The function can return 3 value:
// 0 -> if a and b starts from the same place
// 1 -> if a starts after b
// -1 -> if a starts before b
int cmp_func_adr_of_blocks(void *a, void *b)
{
	block_t *block_a = (block_t *)a;
	block_t *block_b = (block_t *)b;

	if (block_a->start_adr == block_b->start_adr)
		return 0;
	if (block_a->start_adr > block_b->start_adr)
		return 1;
	return -1;
}

// Compare the number of bytes from blocks a and b.
// The function can return 3 value:
// 0 -> if a and b have the same size
// 1 -> if a has more bytes than b
// -1 -> if a has less bytes than b
int cmp_func_size_of_blocks(void *a, void *b)
{
	block_t *block_a = (block_t *)a;
	block_t *block_b = (block_t *)b;

	if (block_a->nr_bytes == block_b->nr_bytes)
		return 0;
	if (block_a->nr_bytes > block_b->nr_bytes)
		return 1;
	return -1;
}

// Return the number of the bytes which are saved
// in the given vector of lists.
u_int count_bytes(adll_t *v)
{
	// Verify the argument.
	if (!v) {
		fprintf(stderr, "count_bytes() - v can not be null\n");
		return 0;
	}

	u_int cnt = 0;
	for (u_int i = 0; i < v->size; ++i) {
		dll_t *list = v->list[i];
		dll_node_t *first_node = list->head;
		block_t *first_block = (block_t *)first_node->data;
		cnt += list->size * first_block->nr_bytes;
	}
	return cnt;
}

// Return the number of the blocks which are
// saved in the given vector of lists.
u_int  count_blocks(adll_t *v)
{
	// Verify the argument.
	if (!v) {
		fprintf(stderr, "count_blocks() - v can not be null\n");
		return 0;
	}

	u_int cnt = 0;
	for (u_int i = 0; i < v->size; ++i)
		cnt += v->list[i]->size;
	return cnt;
}

// Return the value of received pointer in decimal format.
u_int convert_pointer_to_int(void *x)
{
	// We start with a null address.
	void *curr_adr = (void *)0x0;
	// n will save the equivalent of previous address in decimal format.
	u_int n = 0;

	// We could have increased with 1 the values of variables
	// declared before until curr_adr is equal with the received pointer.
	// This method takes a lot of time. A faster one, is to increment them
	// with a neither small, neithar big number, like 100, until we get over
	// received address. After, will go back with the pass -1 until we find
	// the wanted address.
	while (curr_adr < x) {
		curr_adr = (char *)curr_adr + 100;
		n += 100;
	}
	while (curr_adr != x) {
		curr_adr = (char *)curr_adr - 1;
		--n;
	}

	return n;
}

// Returns the smallest start address of a block which is saved
// in the given vector of lists.
// The blocks from every list must be ordered after the start adress.
void *find_min_adr(adll_t *v)
{
	void *min = (void *)0xffffffff;
	for (unsigned int i = 0; i < v->size; ++i) {
		dll_t *list = v->list[i];
		dll_node_t *head = list->head;
		block_t *b_info = (block_t *)head->data;

		if (b_info->start_adr < min)
			min = b_info->start_adr;
	}

	return min;
}

// Returns 1, if the block of memory, determined by the start address and
// by the length specified, has the all bytes in the given vector +
// exists in vector a zone of memory which start from the same address with
// that block.
// Returns 0, if the two previous conditions aren't met.
// The blocks from every list must be ordered after the start address.
u_int block_is_in_vector(adll_t *v, void *start_adr, u_int nr_bytes)
{
	void *stop_adr = (char *)start_adr + nr_bytes;
	// Initially, we consider 0 bytes being in vector from the
	// zone of memory which we want to verify.
	u_int found_bytes = 0;
	// ok will become 1 if exists a block of memory which has
	// the same start adress with the zone which we verify
	u_int ok = 0;

	// We go through the vector of lists which contains the blocks of memory.
	// The purpouse is to find the number of bytes which are in it
	// from the chunk of memory which was given to the function.
	for (u_int i = 0; i < v->size; ++i) {
		// We got to the ith list from vector.
		dll_t *list = v->list[i];

		// We go through the blocks of the ith list until we
		// reach the end of list or get by the last adress in which we
		// are interested. We can do this because the chunk of memories
		// from a list are ordered increasingly after the adress of start.
		dll_node_t *curr_node = list->head;
		block_t *b_info = (block_t *)curr_node->data;
		while (curr_node && b_info->start_adr < stop_adr) {
			// Verify if we have the current block has common
			// bytes with the zone which we verify.
			if (b_info->start_adr >= start_adr) {
				u_int B_per_block = b_info->nr_bytes;
				// to_stop = number of bytes from the start of current
				// block until the end of the zone which we verify
				u_int to_stop = stop_adr - b_info->start_adr;
				found_bytes += B_per_block < to_stop ? B_per_block : to_stop;

				if (b_info->start_adr == start_adr)
					ok = 1;
			}

			curr_node = curr_node->next;
			if (curr_node)
				b_info = (block_t *)curr_node->data;
		}
	}

	if (nr_bytes != found_bytes || !ok)
		return 0;
	return 1;
}

// Takes out the blocks from the given vector, if they start from the specified
// zone. Those blocks are put in another vector which will be returned by
// function. The blocks from every list must be ordered after the start adress.
adll_t *get_blocks_from_zone(adll_t *v, void *start_zone, void *stop_zone)
{
	// Verify the first argument.
	if (!v)
		return NULL;

	// Create the vector of lists which will be returned.
	adll_t *res = adll_create(10, cmp_func_size_of_blocks);
	// We go through the received vector.
	for (u_int i = 0; i < v->size; ++i) {
		// We got to the ith list.
		dll_t *list = v->list[i];

		// We go through the blocks of the ith list until we
		// reach the end of list or get by the last address in which we
		// are interested. We can do this because the chunk of memories
		// from a list are ordered increasingly after the address of start.
		dll_node_t *curr_node = list->head;
		block_t *b_info = curr_node->data;
		// j represents the position of current node in the list
		u_int j = 0;
		while (curr_node && b_info->start_adr < stop_zone) {
			// Verify if the current block starts from the specified zone.
			if (b_info->start_adr >= start_zone) {
				// Add the block in the vector which will be returned.
				int (*cmp)(void *, void *) = &cmp_func_adr_of_blocks;
				adll_add_nd_in_ord(res, b_info, sizeof(block_t), cmp);
				// Go at the next block from the current list.
				curr_node = curr_node->next;
				if (curr_node)
					b_info = curr_node->data;
				// Eliminate the block which was put in the result vector,
				// from the list.
				dll_node_t *removed_node = dll_remove_nth_node(list, j);
				free_dll_node(removed_node);
				// We don't increment j because when the block was removed
				// the others shifted with one position to left.
			} else {
				curr_node = curr_node->next;
				if (curr_node)
					b_info = curr_node->data;
				++j;
			}
		}
		// If the list remained without nodes, we eliminate it from vector.
		if (list->size == 0) {
			adll_remove_nth_list(v, i);
			--i;
		}
	}

	return res;
}

// Remove the block which starts from the specified address
// from the given vector of lists.
// The blocks from every list must be ordered after the start adress.
void remove_block_from_array(adll_t *v, void *start_adr)
{
	// We go through the vetor.
	for (u_int i = 0; i < v->size; ++i) {
		// We got to the ith list.
		dll_t *list = v->list[i];

		// We go through the blocks of the ith list until we
		// reach the end of list or get by the last address in which we
		// are interested. We can do this because the chunk of memories
		// from a list are ordered increasingly after the address of start.
		dll_node_t *curr_node = list->head;
		block_t *b_info = (block_t *)curr_node->data;
		u_int pos = 0;
		while (curr_node && b_info->start_adr <= start_adr) {
			// If the current block start from the specified addres,
			// we eleminate it from the list and stop the function.
			// If not, we go at the next block from the list.
			if (b_info->start_adr == start_adr) {
				curr_node = dll_remove_nth_node(list, pos);
				free_dll_node(curr_node);
				if (list->size == 0)
					adll_remove_nth_list(v, i);
				return;
			}
			curr_node = curr_node->next;
			if (curr_node)
				b_info = curr_node->data;
			pos++;
		}
	}
}

// A block is neighbor with other if can get to it just using the zones
// from the vector. This function reunite the block with all
// its neigbours. All zones which are combined with that block
// will be taken out from the vector of lists.
// The second argument, will send to the user the informations
// about the chunck which resulted form the given block.
// The blocks from every list must be ordered after the start adress.
void add_neighbours_to_block(adll_t *v, block_t *b_info)
{
	// We save the informations about the given block in the next
	// variables. The purpouse is as the next lines to be easier
	// to be followed.
	void *start_adr = b_info->start_adr;
	u_int nr_bytes = b_info->nr_bytes;
	void *stop_adr = (char *)start_adr + nr_bytes;

	// If at the final of while ok is 1, it means that the block
	// was reunited with a neighbour and we must remain in while
	// because a neighboour can also have a neighbour.
	u_int ok = 1;
	while (ok) {
		ok = 0;

		// We go through the vector until its end or until a neighbour
		// was found.
		for (u_int i = 0; i < v->size && !ok; ++i) {
			// We got to the ith list.
			dll_t *list = v->list[i];

			// We go through the nodes from the current list until we
			// reach the end of list or get by the last address in which we
			// are interested. We can do this because the chunk of memories
			// from a list are ordered increasingly after the address of start.
			dll_node_t *curr_node = list->head;
			block_t *curr_b_info = curr_node->data;
			u_int pos = 0;
			while (curr_node && b_info->start_adr <= stop_adr) {
				void *start_block = curr_b_info->start_adr;
				void *stop_block = start_block + curr_b_info->nr_bytes;
				// Verify if it's neighbour at right.
				if (start_block == stop_adr) {
					stop_adr += curr_b_info->nr_bytes;
					nr_bytes += curr_b_info->nr_bytes;
					ok = 1;
				}
				// Verify if it's neighbour at left.
				if (stop_block == start_adr) {
					start_adr -= curr_b_info->nr_bytes;
					nr_bytes += curr_b_info->nr_bytes;
					ok = 1;
				}

				// If it's neighbour, we must take out the block from list.
				// If it's not, we go at the next block.
				if (ok) {
					dll_node_t *rmv_node = dll_remove_nth_node(list, pos);
					free_dll_node(rmv_node);
					// Eliminate the list from the vector, if remained
					// without nodes.
					if (list->size == 0)
						adll_remove_nth_list(v, i);
					// We exit from the current while and after will exit from
					// for because ok is 1. So when we find a neighbour we end
					// the first while's loop.
					break;
				}
				curr_node = curr_node->next;
				if (curr_node)
					curr_b_info = curr_node->data;
				pos++;
			}
		}
	}

	// We update the infoomation about the block, after was reunited with
	// its neighbours.
	b_info->start_adr = start_adr;
	b_info->nr_bytes = nr_bytes;
}
