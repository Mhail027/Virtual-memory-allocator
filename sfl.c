// Copyright Necula Mihail 313CAa 2023-2024
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "list.h"
#include "block.h"

#define u_int unsigned int

// In this struct we'll save the informtaions about
// the allocation's operations.
typedef struct all_cnt_t {
	u_int malloc_calls;
	u_int free_calls;
	u_int fragmentations;
} all_cnt_t;

void DIE(void *p, char *message)
{
	if (!p) {
		fprintf(stderr, "%s", message);
		exit(1);
	}
}

// Allocate dinamicly memory, initialize with 0 and return an
// element of type all_cnt_t.
all_cnt_t *all_cnt_create(void)
{
	all_cnt_t *v = (all_cnt_t *)malloc(sizeof(all_cnt_t));
	DIE(v, "malloc() failed\n");
	v->malloc_calls = 0;
	v->free_calls = 0;
	v->fragmentations = 0;
	return v;
}

// Initialize the heap and return a structure of an vector of double
// linking lists whcih contains the informations about the free blocks
// from heap.
adll_t *INIT_HEAP(void *start_adr, u_int nr_lists, u_int bytes_per_list)
{
	// Create the structure.
	adll_t *v = adll_create(nr_lists, cmp_func_size_of_blocks);

	// Initialize the data for the first node which will be put in vector.
	block_t b_info;
	b_info.start_adr = start_adr;
	b_info.value = NULL;
	b_info.nr_bytes = 8;

	for (u_int i = 0; i < nr_lists; ++i) {
		// Create the ith list.
		dll_t *new_list = dll_create(sizeof(block_t), cmp_func_adr_of_blocks);

		// Add the necesarry nodes in the list.
		for (u_int j = 0; j < bytes_per_list / b_info.nr_bytes; ++j) {
			dll_add_nth_node(new_list, new_list->size, &b_info);
			b_info.start_adr = (char *)b_info.start_adr + b_info.nr_bytes;
		}

		// Add the list in the vector.
		v->list[i] = new_list;
		++v->size;

		// Prepare for the next list.
		b_info.nr_bytes *= 2;
	}

	// Return the structure.
	return v;
}

// Allocates a block of memory with the specified number of bytes.
// The vectors of lists must be ordered inscreasingly after the number
// of bytes from a block. The nodes in every list must be ordered
// increasingly after the address of start. The function can return
// 3 values, depending of how the allocation occured:
// 0 -> MALLOC() failed
// 1 -> MALLOC() succeded
// 2 -> MALLOC() succeded, but fragmentation needed
u_int MALLOC(adll_t *free_dll, adll_t *all_dll, u_int nr_bytes)
{
	// Search the position of the first list which contains blocks
	// of more or same number of free bytes with that asked.
	u_int index = 0;
	for (; index < free_dll->size; ++index) {
		dll_node_t *first_node = free_dll->list[index]->head;
		block_t *first_block = (block_t *)first_node->data;
		if (first_block->nr_bytes >= nr_bytes)
			break;
	}

	// Verify if exists this type of list.
	if (index == free_dll->size) {
		printf("Out of memory\n");
		return 0;
	}

	// Take the first block of memory which is saved in the found list
	// because the nodes are ordered increasingly after the adress of start.
	dll_node_t *taken_node = dll_remove_nth_node(free_dll->list[index], 0);
	void *start_adr = ((block_t *)taken_node->data)->start_adr;
	u_int total_bytes = ((block_t *)taken_node->data)->nr_bytes;
	free_dll_node(taken_node);
	// If the list remains without blocks, we remove it.
	if (!free_dll->list[index]->size)
		adll_remove_nth_list(free_dll, index);

	// Add the zone found in the vector which saves the allocated blocks.
	block_t b_info;
	b_info.start_adr = start_adr;
	b_info.value = NULL;
	b_info.nr_bytes = nr_bytes;
	u_int data_size = sizeof(block_t);
	adll_add_nd_in_ord(all_dll, &b_info, data_size, cmp_func_adr_of_blocks);

	// Calculate what remains after the asked number of bytes are
	// taken from the block of memory.
	b_info.start_adr = (char *)start_adr + nr_bytes;
	b_info.nr_bytes = total_bytes - nr_bytes;
	// If we have bytes which are not allocated, we bring the reaminning
	// block back in the vector of free zones.
	if (b_info.nr_bytes) {
		u_int dt_size = sizeof(block_t);
		adll_add_nd_in_ord(free_dll, &b_info, dt_size, cmp_func_adr_of_blocks);
		return 2;
	}
	return 1;
}

// Take out the block, which starts at the given address, from the vector of
// allocated zone and put it in the vector of free chunks. During the
// proccess of puting the block in the second array, the informations about
// it are copied and is not put an addres at them. The function returns
// the initially structure in which are the informations about the
// block which was freed. It's the responsability of user to free
// its memory.
block_t *FREE_part1(adll_t *all_dll, adll_t *free_dll, void *adr)
{
	// We go through the vector of allocated blocks until we
	// find a zone with the given address,
	for (u_int i = 0; i < all_dll->size; ++i) {
		// We got to the ith list.
		dll_t *list = all_dll->list[i];

		// We goes through the nodes of ith list.
		dll_node_t *curr_node = list->head;
		block_t *b_info = (block_t *)curr_node->data;
		for (u_int j = 0; j < list->size; ++j) {
			// Verify if we found the wanted block.
			if (b_info->start_adr == adr) {
				// Take out the node which contains the block from
				// the vector of allocated zone.
				curr_node = dll_remove_nth_node(list, j);
				free(curr_node);
				// If, after we took out the node which contains the block,
				// the list become empty, we eliminate it from the vector.
				if (!list->size)
					adll_remove_nth_list(all_dll, i);
				// A block which is freed does not need a value.
				free(b_info->value);
				b_info->value = NULL;
				// We put the block in the vector of free zones.
				int (*cmp)(void *, void *) = &cmp_func_adr_of_blocks;
				adll_add_nd_in_ord(free_dll, b_info, sizeof(block_t), cmp);

				return b_info;
			}

			// We go at the next node from the current list.
			curr_node = curr_node->next;
			if (curr_node)
				b_info = (block_t *)curr_node->data;
		}
	}

	return NULL;
}

// If the given block was broken from a zone, when was allocated, is
// reunited with the free chunks from that zone, with which is neighbour.
//	bpl -> bytes per list - is taken from the initialization of heap
void FREE_part2(adll_t *free_dll, adll_t *all_dll, block_t *b_info, u_int bpl)
{
	void *start_adr = b_info->start_adr;

	// Firstly, we must detremine the zone from where the block was taken.
	// To do that, we must reconstituate the initial heap.

	void *min_adr_free = find_min_adr(free_dll);
	void *min_adr_all = find_min_adr(all_dll);
	void *start_heap = min_adr_free;
	if (min_adr_all < start_heap)
		start_heap = min_adr_all;

	// nr_list represint the number of list, from the moment when
	// the heap was initialized, from where the block is coming
	int adr_bk = convert_pointer_to_int(start_adr);
	int adr_heap = convert_pointer_to_int(start_heap);
	u_int nr_list = (adr_bk - adr_heap) / bpl;
	void *start_list = (char *)start_heap + (nr_list * bpl);

	u_int bytes_per_zone = pow(2, 3 + nr_list);
	int adr_list = convert_pointer_to_int(start_list);
	u_int nr_zone = (adr_bk - adr_list) / bytes_per_zone;

	void *start_zone = start_list + bytes_per_zone * nr_zone;
	void *stop_zone = start_zone + bytes_per_zone;

	// Now that we found the zone, we take out all blocks from that
	// zone which are in the vector of free blocks and put them in another
	// vector of blocks.
	adll_t *tmp = get_blocks_from_zone(free_dll, start_zone, stop_zone);

	// We take out, from tmp vector, the block received by function.
	remove_block_from_array(tmp, start_adr);

	// We reunite the block with his neighbours / brothers.
	add_neighbours_to_block(tmp, b_info);

	// Now, the block can be put in the vector of free chunks.
	int (*cmp)(void *, void *) = &cmp_func_adr_of_blocks;
	adll_add_nd_in_ord(free_dll, b_info, sizeof(block_t), cmp);

	// Bring back in the vector of free chunks the blocks which was not used,
	// the blocks which did not renuite with the block which was received by
	// function.
	adll_unify(free_dll, tmp);

	// Free the memory allocated dinamicly in this function.
	adll_free(tmp);
}

// Free the allocated block which start from the given address.
// bpl -> bytes per list - is taken from the initialization of heap
// and is used just if the free is of type 1
// The function returns 1 if the operation of free was done successfully
// or 0 in the other case.
u_int FREE(adll_t *all_dll, adll_t *free_dll, void *adr, u_int type, u_int bpl)
{
	if (!adr)
		return 1;

	// Take thew block which start at given addres from the vector
	// with allocated zones and put it in the vector with freee chunks.
	block_t *freed_block = FREE_part1(all_dll, free_dll, adr);

	if (!freed_block) {
		printf("Invalid free\n");
		return 0;
	}

	if (!type) {
		free(freed_block);
		return 1;
	}

	// Reunite the freed block with the zone from where is coming.
	FREE_part2(free_dll, all_dll, freed_block, bpl);
	free(freed_block);

	return 1;
}

// Print the address of start and the number of bytes of every
// block of memory saved in the given vector of lists.
// The blocks will be printed grouped after the number of bytes.
// The zones will be entitled as being free chunks.
// The vectors of lists must be ordered inscreasingly after the number
// of bytes from a block. The nodes in every list must be ordered
// increasingly after the address of start
void print_free_blocks(adll_t *v)
{
	// Verify the argument.
	if (!v) {
		fprintf(stderr, "print_free_blocks() - v can't be null\n");
		return;
	}

	for (u_int i = 0; i < v->size; ++i) {
		dll_t *list = v->list[i];
		dll_node_t *curr_node = list->head;
		block_t *b_info = (block_t *)curr_node->data;

		printf("Blocks with %u bytes", b_info->nr_bytes);
		printf(" - ");
		printf("%u free block(s) :", list->size);
		for (u_int j = 0; j < list->size; ++j) {
			printf(" %p", b_info->start_adr);
			curr_node = curr_node->next;
			if (curr_node)
				b_info = (block_t *)curr_node->data;
		}
		printf("\n");
	}
}

// Print the address of start and the number of bytes of every
// block of memory saved in the given vector of lists.
// The blocks will be printed increasingly after the address of start.
// The zones will be entitled as being allocated chunks.
// The nodes in every list must be ordered increasingly after the address
// of start
void print_all_blocks(adll_t *v)
{
	// Verify the argument.
	if (!v) {
		fprintf(stderr, "print_all_blocks() - v can't be null\n");
		return;
	}

	printf("Allocated blocks :");

	// We create an vector of nodes in which save the first node
	// from evrey list.
	u_int size_vector = v->size * sizeof(dll_node_t *);
	dll_node_t **curr_node_in_list = (dll_node_t **)malloc(size_vector);
	DIE(curr_node_in_list, "malloc() failed\n");
	for (u_int i = 0; i < v->size; ++i)
		curr_node_in_list[i] = v->list[i]->head;

	do {
		// Find the block which is placed
		// the closest to 0x0 from the created vector.
		int i_min = -1;
		void *min = (void *)0xffffffff;
		for (u_int i = 0; i < v->size; ++i)
			if (curr_node_in_list[i]) {
				block_t *b_info = curr_node_in_list[i]->data;
				if (b_info->start_adr <= min) {
					min = b_info->start_adr;
					i_min = i;
				}
			}

		// Verify if we have not-null blocks in the vector.
		if (i_min == -1)
			break;

		// Print the informations about the block which we found.
		printf(" (%p", min);
		printf(" - ");
		printf("%u)",  ((block_t *)curr_node_in_list[i_min]->data)->nr_bytes);

		// After printing a block from the list, we must go at the next one.
		curr_node_in_list[i_min] = curr_node_in_list[i_min]->next;
	} while (1);

	printf("\n");

	// Free the memory which was allocate dinamicly inside the function.
	free(curr_node_in_list);
}

// Display on the screen the infromations about the memory of heap which
// we initialized and about the proccess of allocation which we made.
void DUMP_MEMORY(adll_t *free_dll, adll_t *all_dll, all_cnt_t *all_cnt)
{
	printf("+++++DUMP+++++\n");

	u_int free_bytes = count_bytes(free_dll);
	u_int all_bytes = count_bytes(all_dll);
	u_int total_bytes = free_bytes + all_bytes;
	printf("Total memory: %u bytes\n", total_bytes);
	printf("Total allocated memory: %u bytes\n", all_bytes);
	printf("Total free memory: %u bytes\n", free_bytes);

	u_int free_blocks = count_blocks(free_dll);
	u_int all_blocks = count_blocks(all_dll);
	printf("Free blocks: %u\n", free_blocks);
	printf("Number of allocated blocks: %u\n", all_blocks);

	printf("Number of malloc calls: %u\n", all_cnt->malloc_calls);
	printf("Number of fragmentations: %u\n", all_cnt->fragmentations);
	printf("Number of free calls: %u\n", all_cnt->free_calls);

	print_free_blocks(free_dll);
	print_all_blocks(all_dll);

	printf("-----DUMP-----\n");
}

// Convert a string to a positive integer and return that number.
u_int str_to_int(char *str)
{
	u_int nr = 0;
	while (str[0]) {
		nr = nr * 10 + str[0] - '0';
		str++;
	}

	return nr;
}

// Write the given data in a zone of memory which has the start adress
// and the length specified.
// In all_dll we have the allocated blocks.
// The blocks from every list must be ordered after the start adress.
// The function returns 1 if the operation of write was done successfully
// or 0 in the other case.
u_int WRITE(adll_t *all_dll, char *data, void *start_adr, u_int nr_bytes)
{
	// Verify if the zone in which we want to write has all
	// bytes allocated.
	if (!block_is_in_vector(all_dll, start_adr, nr_bytes)) {
		printf("Segmentation fault (core dumped)\n");
		return 0;
	}

	void *stop_adr = (char *)start_adr + nr_bytes;
	// We go through the vector of lists which contains the allocated blocks.
	// When we find a part from the zone in which we work, we write in it
	// the correspomding segment from the string which was received by
	// function.
	for (u_int i = 0; i < all_dll->size; ++i) {
		// We got to the ith list from vector.
		dll_t *list = all_dll->list[i];

		// We go through the blocks of the ith list until we reach the
		// end of list or get by the last address in which we are interested.
		// We can do this because the chunk of memories from a list are ordered
		// increasingly after the address of start.
		dll_node_t *curr_node = list->head;
		block_t *b_info = curr_node->data;
		while (curr_node && b_info->start_adr < stop_adr) {
			if (b_info->start_adr >= start_adr) {
				// B_per_block = number of bytes from the current block
				u_int B_per_block = b_info->nr_bytes;
				// to_stop = number of bytes from the start of current
				// block until the end of the zone in which we want to write
				u_int to_stop = stop_adr - b_info->start_adr;
				// cp_bytes represent the number of bytes which will be
				// copied from data in the current block
				u_int cp_bytes = B_per_block < to_stop ? B_per_block : to_stop;

				// Verify if the field data from the current node has enough
				// allocated bytes for the text which will be copied in it.
				char *curr_value = b_info->value;
				if (!curr_value || cp_bytes > strlen(curr_value)) {
					if (!curr_value)
						free(curr_value);
					b_info->value = calloc(cp_bytes + 1, sizeof(char));
					DIE(b_info->value, "calloc() failed\n");
				}

				// To get to the part of text which we want to copy,
				// we must determine the offset.
				u_int offset = b_info->start_adr - start_adr;

				// Now, the copy can take place.
				strncpy(b_info->value, data + offset, cp_bytes);
			}

			curr_node = curr_node->next;
			if (curr_node)
				b_info = curr_node->data;
		}
	}

	return 1;
}

// Read and print on the screen what is saved in the zone of memory
// which has the start adress and the length specified.
// In all_dll we have the allocated blocks.
// The blocks from every list must be ordered after the start adress.
// The function returns 1 if the operation of read was done successfully
// or 0 in the other case.
u_int READ(adll_t *all_dll, void *start_adr, u_int nr_bytes)
{
	// Verify if the zone which we want to read has all
	// bytes allocated.
	if (!block_is_in_vector(all_dll, start_adr, nr_bytes)) {
		printf("Segmentation fault (core dumped)\n");
		return 0;
	}

	void *stop_adr = (char *)start_adr + nr_bytes;
	// data <- the string saved in the given zone
	char *data = (char *)calloc(nr_bytes + 1, sizeof(char));
	DIE(data, "calloc() failed\n");
	// We go through the vector of lists which contains the allocated blocks.
	// When we find the block who starts from the zone in which we want to
	// read, we stop there. Concatante to data the value of that block
	// and update the zone from where we want to read (update start_adr).
	// After, we go again through this process until we read all the initial
	// zone.
	do {
		for (u_int i = 0; i < all_dll->size; ++i) {
			// We got to the ith list from vector.
			dll_t *list = all_dll->list[i];

			// We go through the blocks of the ith list until we reach the
			// end of list or get by the last address in which we are
			// interested. We can do this because the chunk of memories
			// from a list are ordered increasingly after the address of start.
			dll_node_t *curr_node = list->head;
			block_t *b_info = curr_node->data;
			while (curr_node && b_info->start_adr < stop_adr) {
				if (b_info->start_adr == start_adr) {
					// B_per_block = number of bytes from the current block
					u_int B_per_block = b_info->nr_bytes;
					// to_stop = number of bytes from the start of current
					// block until the end of the zone from which we want to
					// read
					u_int to_stop = stop_adr - b_info->start_adr;
					// cp_bytes represent the number of bytes which will be
					// copied from curent block in data
					u_int cp_bytes = 0;
					cp_bytes = B_per_block < to_stop ? B_per_block : to_stop;

					// We add at the final of data, the part of information,
					// in which we are intereseted, from the current block.
					strncpy(data + strlen(data), b_info->value, cp_bytes);

					// Update the zone from where we want to read.
					start_adr += b_info->nr_bytes;
					nr_bytes -= cp_bytes;

					// Stop the for.
					i = all_dll->size;
					break;
				}

				curr_node = curr_node->next;
				if (curr_node)
					b_info = curr_node->data;
			}
		}
	} while (start_adr < stop_adr);

	// Print the string which is saved in the given zone of memory.
	printf("%s\n", (char *)data);
	// Free the memory which was allocate dinamicly inside the function.
	free(data);
	return 1;
}

// Free all the memory which was allocated for keeping the informations
// about heap and about allocations's operations. Also, stop the
// running of the program.
void DESTROY_HEAP(adll_t *free_dll, adll_t *all_dll, all_cnt_t *all_cnt)
{
	// Because the field data of the node with which we work send us
	// to a complex type (block_t), before to call the function which
	// free the memory allocated for an array of lists, it's our responsaility
	// to free any variable alloacted dinamicly at which data send us.
	// In our case, in block_t, just the value is allocated dinaamicly.

	// Free the memory allocated for the structure which
	// save the free blocks.
	for (u_int i = 0; i < free_dll->size; ++i) {
		dll_node_t *curr_node = free_dll->list[i]->head;
		while (curr_node) {
			block_t *b_info = curr_node->data;
			free(b_info->value);
			curr_node = curr_node->next;
		}
	}
	adll_free(free_dll);

	// Free the memory allocate for the structure which
	// save the allocated  blocks.
	for (u_int i = 0; i < all_dll->size; ++i) {
		dll_node_t *curr_node = all_dll->list[i]->head;
		while (curr_node) {
			block_t *b_info = curr_node->data;
			free(b_info->value);
			curr_node = curr_node->next;
		}
	}
	adll_free(all_dll);

	// Free the memory allocated for the structure which
	// save the informations about alocations's operations.
	free(all_cnt);

	// Stop the program.
	exit(0);
}

int main(void)
{
	adll_t *free_dll = NULL;
	// all -> allocated / allocations
	adll_t *all_dll = adll_create(16, cmp_func_size_of_blocks);
	all_cnt_t *all_cnt = all_cnt_create();

	u_int type, bytes_per_list;
	char *main_command = (char *)malloc(101 * sizeof(char));
	DIE(main_command, "malloc() failed\n");

	while (1) {
		scanf("%s", main_command);

		if (!strcmp(main_command, "INIT_HEAP")) {
			void *adr_heap;
			u_int nr_lists;
			scanf("%p%u%u%u", &adr_heap, &nr_lists, &bytes_per_list, &type);
			free_dll = INIT_HEAP(adr_heap, nr_lists, bytes_per_list);
		}

		if (!strcmp(main_command, "MALLOC")) {
			u_int nr_bytes;
			scanf("%u", &nr_bytes);
			u_int x = MALLOC(free_dll, all_dll, nr_bytes);
			if (x == 1 || x == 2)
				++all_cnt->malloc_calls;
			if (x == 2)
				++all_cnt->fragmentations;
		}

		if (!strcmp(main_command, "FREE")) {
			void *adr;
			scanf("%p", &adr);
			u_int x = FREE(all_dll, free_dll, adr, type, bytes_per_list);
			if (x)
				all_cnt->free_calls++;
		}

		if (!strcmp(main_command, "DUMP_MEMORY"))
			DUMP_MEMORY(free_dll, all_dll, all_cnt);

		if (!strcmp(main_command, "DESTROY_HEAP")) {
			free(main_command);
			DESTROY_HEAP(free_dll, all_dll, all_cnt);
		}

		if (!strcmp(main_command, "WRITE")) {
			void *start_adr;
			scanf("%p", &start_adr);
			char *buffer = (char *)malloc(701 * sizeof(char));
			DIE(buffer, "malloc() failed\n");
			fgets(buffer, 701, stdin);
			char *string = strtok(buffer, "\"");
			string = strtok(NULL, "\"");
			u_int nr_bytes = str_to_int(string + strlen(string) + 2);
			if (nr_bytes > strlen(string))
				nr_bytes = strlen(string);

			if (!WRITE(all_dll, string, start_adr, nr_bytes)) {
				DUMP_MEMORY(free_dll, all_dll, all_cnt);
				free(buffer);
				free(main_command);
				DESTROY_HEAP(free_dll, all_dll, all_cnt);
			}
			free(buffer);
		}

		if (!strcmp(main_command, "READ")) {
			void *start_adr;
			u_int nr_bytes;
			scanf("%p%d", &start_adr, &nr_bytes);

			if (!READ(all_dll, start_adr, nr_bytes)) {
				DUMP_MEMORY(free_dll, all_dll, all_cnt);
				free(main_command);
				DESTROY_HEAP(free_dll, all_dll, all_cnt);
			}
		}
	}
	return 0;
}
