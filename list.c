// Copyright Necula Mihail 313CAa 2023-2024
#include "list.h"

static void
DIE(void *p, char *message)
{
	if (!p) {
		fprintf(stderr, "%s", message);
		exit(1);
	}
}

// Free the memory allocated for a node from a doubly linked list,
// without to take in consideration that data can have in it other
// things which should be freed.
void free_dll_node(dll_node_t *node)
{
	free(node->data);
	free(node);
}

// Create, initialize and return a doubly linked list.
dll_t *dll_create(u_int data_size, int (*compare_function)(void *, void *))
{
	// Allocate memory for the list./ Create the list.
	dll_t *list = (dll_t *)malloc(sizeof(dll_t));
	DIE(list, "malloc() failed\n");

	// Initialize the list.
	list->head = NULL;
	list->data_size = data_size;
	list->size = 0;
	list->compare_function = compare_function;

	// Return the list.
	return list;
}

// Return the address of nth node from given doubly linked list.
// The count starts from 0, so n must be in the next interval:
// [0, size of the list - 1].
dll_node_t *dll_get_nth_node(dll_t *list, u_int n)
{
	// Veriy the arguments.
	if (!list) {
		fprintf(stderr, "dll_get_nth_node() - list can't be NULL\n");
		return NULL;
	}
	if (!list->head) {
		fprintf(stderr, "dll_get_nth_node() - list must have at least a node");
		return NULL;
	}
	if (n >= list->size) {
		fprintf(stderr, "dll_get_nth_node() - the index n is not valid\n");
		return NULL;
	}

	// We go throught the list until get to the nth node.
	dll_node_t *curr_node = list->head;
	for (u_int i = 1; i <= n; ++i)
		curr_node = curr_node->next;

	// Return the nth node.
	return curr_node;
}

// Remove the nth node from a doubly linked list and return the address of
// that node. It is the responsability of user to free its memory.
// The position is indexed from 0. If the position is bigger or equal
// with the size of list, we will remove the last node.
dll_node_t *dll_remove_nth_node(dll_t *list, u_int n)
{
	// Verify the arguments.
	if (!list) {
		fprintf(stderr, "dll_remove_nth_node() - list can't be NULL\n");
		return NULL;
	}
	if (!list->head) {
		fprintf(stderr, "dll_remove_nth_node() - list must have a head\n");
		return NULL;
	}
	if (n >= list->size)
		n = list->size - 1;

	// Find the nth node.
	dll_node_t *curr_node = dll_get_nth_node(list, n);

	// If has a previous node, we break the link with it.
	if (curr_node->prev)
		curr_node->prev->next = curr_node->next;
	// If has a next node, we break the link with it.
	if (curr_node->next)
		curr_node->next->prev = curr_node->prev;
	// If we eliminate the head of list, we must update from
	// where the list starts.
	if (curr_node == list->head)
		list->head = curr_node->next;

	// Decrement the size of list.
	list->size--;

	// Return the removed node.
	return curr_node;
}

// Create, initialize and put a node on nth postion in the given list.
// The position is indexed from 0.
// If the position n is bigger than the size of the list, the node is added
// at end.
void dll_add_nth_node(dll_t *list, u_int n, void *data)
{
	// Verify the arguments.
	if (!list) {
		fprintf(stderr, "dll_add_nth_node() - list can't be NULL\n");
		return;
	}
	if (n > list->size)
		n = list->size;

	// Create the new node.
	dll_node_t *new_node = (dll_node_t *)malloc(sizeof(dll_node_t));
	DIE(new_node, "malloc() failed\n");

	// Initialize the new node.
	new_node->data = malloc(list->data_size);
	DIE(new_node->data, "malloc() failed\n");
	memcpy(new_node->data, data, list->data_size);

	// We have 3 places where we can be asked to place the new node.
	// To identify the scenario in which we are, will have 3 ifs.

	// Insert the new node at the begining of the list.
	if (!n) {
		new_node->prev = NULL;
		new_node->next = list->head;
		if (list->head)
			list->head->prev = new_node;
		list->head = new_node;
	}

	// Insert the new node in the middle of the list.
	if (n >= 1 && n <= list->size - 1) {
		dll_node_t *prev_node = dll_get_nth_node(list, n - 1);
		dll_node_t *next_node = prev_node->next;
		next_node->prev = new_node;
		new_node->next = next_node;
		new_node->prev = prev_node;
		prev_node->next = new_node;
	}

	// Insert the new node at the final of the list.
	// In the case that the list has 0 nodes, add to the final of the list is
	// sysonymus with add to the start of the list.
	// To not add the new node 2 times we must verify that the list does
	// not have 0 nodes.
	if (n == list->size && list->size) {
		dll_node_t *prev_node = dll_get_nth_node(list, n - 1);
		new_node->next = NULL;
		new_node->prev = prev_node;
		prev_node->next = new_node;
	}

	// Increment the size of list.
	++list->size;
}

// Create, initialize, and put a node in the given list in which
// the nodes are ordered incresingly by data using
// the function of compare which the list has it.
// At final, the list will have the nodes still ordered.
void dll_add_node_in_order(dll_t *list, void *data)
{
	// Verify the arguments.
	if (!list) {
		fprintf(stderr, "dll_add_node_in_order() - list can't be NULL");
		return;
	}

	// Find the index of first node which has the data equal or bigger
	// than the data received by function.
	// If does not exist such a node, the index will send
	// at the next position after the end of list.
	int index = 0;
	dll_node_t *curr_node = list->head;
	while (curr_node && list->compare_function(data, curr_node->data) > 0) {
		curr_node = curr_node->next;
		++index;
	}

	// We will add the new node on the indexth position because:
	// -> every previous node has a smaller data
	// and
	// -> every node after the index position, inclusively,
	// has an equall or bigger data
	dll_add_nth_node(list, index, data);
}

// Free the memory allocated for the given doubly linked list.
void dll_free(dll_t *list)
{
	// Free the memory allocated for every node from list.
	dll_node_t *curr_node = list->head;
	while (curr_node) {
		dll_node_t *removed_node = curr_node;
		curr_node = curr_node->next;
		free_dll_node(removed_node);
	}

	// Free the memory allocated for the structure of list.
	free(list);
}

// Create, initialize and return the structure for
// a vector of doubly link lists.
adll_t *adll_create(u_int nr_lists, int (*compare_function)(void *, void *))
{
	// Verify the argument.
	if (!nr_lists) {
		fprintf(stderr, "adll_create() - can't create an vector of size 0\n");
		return NULL;
	}

	// Allocate memory for the structure of vector.
	adll_t *v = (adll_t *)malloc(sizeof(adll_t));
	DIE(v, "malloc() failed\n");

	// Initialize the fields of structure.
	v->list = (dll_t **)malloc(nr_lists * sizeof(dll_t *));
	DIE(v->list, "malloc() failed\n");
	v->size = 0;
	v->max_size = nr_lists;
	v->compare_function = compare_function;

	// Return the structure.
	return v;
}

// Realloc the memory of the vector of doubly linked lists from
// the given structure. The new vector will have new_size elements.
void adll_realloc(adll_t *v, u_int new_size)
{
	//Verify the arguments
	if (!v) {
		fprintf(stderr, "adll_realloc() - v can't be null\n");
		return;
	}
	if (!v->list) {
		fprintf(stderr, "adll_realloc() - vector can't be null\n");
		return;
	}
	if (!v->max_size) {
		fprintf(stderr, "adll_realloc() - size of vector can't be 0\n");
		return;
	}

	// Realloc the memory for the vector.
	v->list = realloc(v->list, sizeof(adll_t) * new_size);
	DIE(v->list, "realloc() failed\n");

	// Actualize the size of vector.
	v->max_size = new_size;
}

// Put the specified list on nth position in the given vector.
// The position is indexed from 0. If n is bigger than the used
// size of vector, the list will be added after the last non-null element.
void adll_add_nth_list(adll_t *v, u_int n, dll_t *list)
{
	//Verify the arguments.
	if (!v) {
		fprintf(stderr, "adll_add_nth_list() - v can't be null\n");
		return;
	}
	if (!list) {
		fprintf(stderr, "adll_realloc() - list can't be null\n");
		return;
	}
	if (n > v->size)
		n = v->size;

	// If the vector does not have a free position, we must
	// realloc its memory to have one. Because would be ineficiently,
	// from the perspective of time, to resize tha vector with +1
	// element every time, we will double its size.
	if (v->size == v->max_size)
		adll_realloc(v, v->max_size * 2);

	// All the elements after the nth position, inclusively,
	// are shifted with one position to the right to make space
	// for the new list which will be added.
	if (v->size)
		for (int i = v->size - 1; i >= (int)n; --i)
			v->list[i + 1] = v->list[i];
	// Add the new list in the vector.
	v->list[n] = list;

	// Increment the used size of the vector.
	++v->size;
}

// Remove the nth list from the vector of give struct.
// The position is indexed from 0. If n is bigger or
// equal with the used size of vector, we'll eliminate
// the last non-null elemet.
void adll_remove_nth_list(adll_t *v, u_int n)
{
	// Verify the arguments.
	if (!v) {
		fprintf(stderr, "adll_remove_nth_list() - v can't be null\n");
		return;
	}
	if (!v->list) {
		fprintf(stderr, "adll_remove_nth_list - vector can't be null\n");
		return;
	}
	if (!v->size) {
		fprintf(stderr, "adll_remove_nth_node() - size of vector can't be 0\n");
		return;
	}
	if (n >= v->size)
		n = v->size - 1;

	// Free the memory allocated to the nth list.
	dll_free(v->list[n]);

	// All the elements after the nth position are shifted with +1
	// to the left.
	for (u_int i = n + 1; i < v->size; ++i)
		v->list[i - 1] = v->list[i];

	// Decrement the used size of vector.
	--v->size;
}

// Create, initialize and put a node in a list from the vector of given
// struct. The nodes are grouped in lists after a common value of a field
// from their data. The lists from vector are ordered increasingly after that
// criterion. The nodes from every list are ordered inscreasingly
// after other criterion. At the final of the function, every list and the
// vector will remain ordered. We'll use the last 2 arguments of the function,
// if we need to create a new list for the node which we'll add in vector.
// I didn't have enough space to give to the arguments intuitive names.
// So, this is what every argument means:
// v -> the vector of lists
// d -> the data of new node
// s -> size of the data
// c -> the compare function for the nodes from a list
void adll_add_nd_in_ord(adll_t *v, void *d, u_int s, int (*c)(void *, void *))
{
	// Verify the arguments.
	if (!v) {
		fprintf(stderr, "adll_add_nd_in_ord() - v can't be null\n");
		return;
	}

	// We search the list in which the data would fit after the
	// criterion of lists's ordering.
	u_int i = 0;
	while (i < v->size) {
		if (!v->compare_function(v->list[i]->head->data, d)) {
			dll_add_node_in_order(v->list[i], d);
			return;
		}
		// If we get over the list which would fit for our node, don't
		// have why to continue the while, because the criterion will
		// go further and further upward.
		if (v->compare_function(v->list[i]->head->data, d) > 0)
			break;
		++i;
	}

	// If we get here, it's means that does not exist a list
	// for the recieved data. We will create it.
	dll_t *new_list = dll_create(s, c);
	dll_add_nth_node(new_list, 0, d);

	// Will add the new created list on the position of vector at we
	// exit from while.
	adll_add_nth_list(v, i, new_list);
}

// Free the memory allocated for the given structure which save a vector
// of doubly linked list.
void adll_free(adll_t *v)
{
	// Free the memory allocate for every list.
	for (u_int i = 0; i < v->size; ++i)
		dll_free(v->list[i]);
	// Free the memory allocated for array.
	free(v->list);
	// Free the memory allocated for structure.
	free(v);
}

// Make a coppy at every node from src and put it in dst.
// At end, dst will be the reunion between dst and src,
// with the posibility to exist duplicates. The second vector is
// not modified.
// The 2 vectors must have the same function of compare. Also the lists
// must have the same function of compare. In plus, the lists must save
// the same type of data.
void adll_unify(adll_t *dst, adll_t *src)
{
	for (u_int i = 0; i < src->size; ++i) {
		dll_t *list = src->list[i];
		int data_size = list->data_size;
		int (*cmp)(void *, void *) = list->compare_function;

		dll_node_t *curr_node = list->head;
		for (u_int j = 0; j < list->size; ++j) {
			adll_add_nd_in_ord(dst, curr_node->data, data_size, cmp);
			curr_node = curr_node->next;
		}
	}
}

