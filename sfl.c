//Ana Stanciulescu, 312CA, 07.04.2024
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)

typedef enum BLOCK_STATE {
	FREE,
	ALLOC
} BLOCK_STATE;

typedef char ALIGN[16];

struct header_t {
	unsigned int heap_size, allocd_bytes, n_free_blocks, n_allocd_blocks;
	int malloc_calls, free_calls, n_fragments;
	int num_lists;
};

static struct header_t header;

//structure for block information
typedef struct info info;
struct info {
	unsigned int data_size;
	char *values;
	int reconstruction_type;
	BLOCK_STATE is_free;
	unsigned long address;
};

//block structure
typedef struct dll_block_t dll_block_t;
struct dll_block_t {
	void *data;
	dll_block_t *prev, *next;
};

//list structure
typedef struct doubly_linked_list_t doubly_linked_list_t;
struct doubly_linked_list_t {
	dll_block_t *head;
	unsigned int data_size; //8, 16, 32,...
	unsigned int size; //total number of blocks in a list
	unsigned long address_start; //the address from which the list starts
};

//function that creates a double-linked list
doubly_linked_list_t *dll_create_list(unsigned int data_size)
{
	doubly_linked_list_t *list = malloc(sizeof(doubly_linked_list_t));
	DIE(!list, "Cannot allocate memory for list");

	list->head = NULL;
	list->size = 0;
	list->data_size = data_size - data_size;
	list->address_start = 0;

	return list;
}

//function that adds a new block to the segregated free list
void dll_add_nth_block_sfl(doubly_linked_list_t *list, unsigned int n,
						   int reconstruction_type, unsigned long address)
{
	dll_block_t *current_block, *previous_block;

	if (!list) {
		DIE(!list, "Initialize the list first");
		return;
	}

	if (n > list->size)
		n = list->size;

	//the new block is created
	dll_block_t *new_block = malloc(sizeof(*new_block));
	new_block->data = malloc(sizeof(info));

	//the block's position is determined in order to link it in the list
	if (!list->head) {
		list->head = new_block;
		list->head->next = NULL;
		list->head->prev = NULL;
		((info *)list->head->data)->address = list->address_start;
	}

	//moves in the list to the specified position
	previous_block = NULL;
	current_block = list->head;
	for (unsigned int i = 0; i < n; i++) {
		previous_block = current_block;
		current_block = current_block->next;
	}

	//copies all the necessary information
	new_block->next = current_block;
	((info *)new_block->data)->address = address;
	((info *)new_block->data)->reconstruction_type = reconstruction_type;
	((info *)new_block->data)->is_free = FREE;
	((info *)new_block->data)->data_size = list->data_size;
	((info *)new_block->data)->values = NULL;

	if (n == 0) {
		list->address_start = address;
		new_block->prev = NULL;
		list->head = new_block;

	} else {
		previous_block->next = new_block;
		new_block->prev = previous_block;
	}
}

//function that adds a block in the allocated-memory list
void dll_add_nth_block_aml(doubly_linked_list_t *list, unsigned int n,
						   void *data, int reconstruction_type,
						   unsigned int data_size, unsigned long address)
{
	dll_block_t *current_block, *previous_block;

	if (!list) {
		DIE(!list, "Initialize the list first");
		return;
	}

	if (n > list->size)
		n = list->size;

	//the new block is created
	dll_block_t *new_block = malloc(sizeof(*new_block));
	new_block->data = malloc(sizeof(info));
	//if the "data" parameter exists, it copies the data into the new block
	if (data)
		memcpy(((info *)new_block->data)->values, data, data_size);
	else
		((info *)new_block->data)->values = NULL;

	((info *)new_block->data)->data_size = data_size;
	//the block's position is determined in order to link it in the list
	if (!list->head) {
		list->head = new_block;
		list->head->next = NULL;
		list->head->prev = NULL;
		((info *)list->head->data)->address = list->address_start;
	}

	//moves to the specified position
	previous_block = NULL;
	current_block = list->head;
	for (unsigned int i = 0; i < n; i++) {
		previous_block = current_block;
		current_block = current_block->next;
	}

	new_block->next = current_block;
	if (n < list->size)
		current_block->prev = new_block;
	if (n == 0) {
		new_block->prev = previous_block;
		list->head = new_block;
	} else {
		previous_block->next = new_block;
		new_block->prev = previous_block;
	}
	//the remaining information is copied into the block
	((info *)new_block->data)->address = address;
	((info *)new_block->data)->reconstruction_type = reconstruction_type;
	((info *)new_block->data)->is_free = ALLOC;
}

//function that removes any type of block from a list
dll_block_t *dll_remove_nth_block(doubly_linked_list_t *list, unsigned int n)
{
	dll_block_t *current_block, *previous_block;

	if (!list) {
		DIE(!list, "Initialize the list first");
		return NULL;
	}

	if (n > list->size - 1)
		n = list->size - 1;

	//moves to the specified position
	current_block = list->head;
	previous_block = NULL;
	for (unsigned int i = 0; i < n; i++) {
		previous_block = current_block;
		current_block = current_block->next;
	}

	//depending on the position, it cuts the links
	if (n == 0) {
		list->head = current_block->next;
		if (n < list->size - 1)
			(current_block->next)->prev = previous_block;
	} else {
		previous_block->next = current_block->next;
		if (n < list->size - 1)
			(current_block->next)->prev = previous_block;
	}

	list->size--;

	return current_block;
}

//function to determine the list's size
unsigned int dll_get_size(doubly_linked_list_t *list)
{
	if (!list)
		return -1;

	return list->size;
}

//function that frees all the blocks from a list and the list itself
void dll_free(doubly_linked_list_t **pp_list)
{
	dll_block_t *current_block;

	if (!pp_list || !(*pp_list)) {
		DIE(!pp_list, "Initialize the list first");
		return;
	}

	unsigned int n = (*pp_list)->size, i;
	//moves through each block, removes it and frees all the data
	for (i = 0; i < n; i++) {
		current_block = dll_remove_nth_block(*pp_list, 0);
		((info *)current_block->data)->data_size = 0;
		((info *)current_block->data)->address = 0;
		if (((info *)current_block->data)->values)
			free(((info *)current_block->data)->values);
		free(current_block->data);
		current_block->data = NULL;
		free(current_block);
		current_block = NULL;
	}

	//frees the list
	free(*pp_list);
	*pp_list = NULL;
}

//function that only prints the address of a block, depending on its type
void dll_print_block_address(doubly_linked_list_t *list)
{
	dll_block_t *current_block;

	if (!list) {
		DIE(!list, "Initialize the list first");
		return;
	}

	current_block = list->head;
	int n = list->size;
	//moves through each block and prints the relevant info
	for (int i = 0; i < n; i++) {
		if (((info *)current_block->data)->is_free == FREE) {
			printf(" 0x%lx", ((info *)current_block->data)->address);
		} else {
			printf(" (0x%lx - %d)", ((info *)current_block->data)->address,
				   ((info *)current_block->data)->data_size);
		}
		current_block = current_block->next;
	}

	printf("\n");
}

//function used to determine the power of a given size
int log_2(unsigned int n)
{
	int exp = 0;
	while (n != 1) {
		n >>= 1;
		exp++;
	}
	return exp;
}

//function that initializes the heap with all its blocks
doubly_linked_list_t **INIT_HEAP(unsigned long start_heap, int num_lists,
								 int num_bytes_per_list, int recon_type)
{
	//determines the length of the array / max number of lists
	unsigned int length = 1 << (2 + num_lists);
	doubly_linked_list_t **seg_list =
						 malloc((length + 1) * sizeof(doubly_linked_list_t *));

	DIE(!seg_list, "Cannot allocate memory for list");

	//initializes the header information (will be useful for the memory dump)
	header.heap_size = num_bytes_per_list * num_lists;
	header.free_calls = 0; header.malloc_calls = 0;
	header.allocd_bytes = 0; header.n_allocd_blocks = 0;
	header.n_free_blocks = 0; header.num_lists = num_lists;

	//creates a list for each size class
	//they all point to NULL
	for (unsigned int i = 0; i < length + 1; i++)
		seg_list[i] = dll_create_list(i);

	//adds blocks to the wanted size classes
	for (unsigned int i = 8; i <= length; i *= 2) {
		seg_list[i]->address_start = start_heap + (log_2(i) - 3) *
									num_bytes_per_list;
		seg_list[i]->data_size = i;
		seg_list[i]->size = num_bytes_per_list / i;
		header.n_free_blocks = header.n_free_blocks + seg_list[i]->size;
		unsigned int n = seg_list[i]->size;
		for (unsigned int j = 0; j < n; j++) {
			dll_add_nth_block_sfl(seg_list[i], j, recon_type,
								  seg_list[i]->address_start + j * i);
		}
	}

	return seg_list;
}

//function that returns the first block that is free and has
//a good size for the malloc function
dll_block_t *dll_get_free_block(doubly_linked_list_t **pp_list,
								unsigned int nr_bytes)
{
	dll_block_t *current_block;
	unsigned int length = 1 << (2 + header.num_lists);

	if (!pp_list) {
		DIE(!pp_list, "Initialize the list first");
		return NULL;
	}

	//goes through the entire array of lists, until it finds a good block
	current_block = NULL;
	for (unsigned int i = nr_bytes; i <= length; i++) {
		if (!pp_list[i])
			continue;
		if (!(pp_list[i]->head))
			continue;
		//the good block will always be the first in a list
		current_block = dll_remove_nth_block(pp_list[i], 0);
		if (pp_list[i]->size == 0)
			pp_list[i]->head = NULL;
		break;
	}
	return current_block;
}

//function that returns the position at which to add a new block
//as to have the list sorted after the block addresses
unsigned int dll_get_pos(doubly_linked_list_t *list, unsigned long address)
{
	if (!list)
		return 0;
	if (!list->head)
		return 0;

	//if the list has only one block,
	//it compares the new address only with the existing block's
	if (list->size == 1) {
		if (((info *)list->head->data)->address > address)
			return 0;
		else
			return 1;
	}

	//compares the given address with current block and the next
	//to determine the position
	dll_block_t *current_block;
	unsigned int pos;
	current_block = list->head;
	if (address < ((info *)current_block->data)->address)
		return 0;

	unsigned int i;
	for (i = 0; i < list->size - 1; i++) {
		if (((info *)current_block->data)->address <= address &&
			((info *)current_block->next->data)->address >= address) {
			pos = i + 1;
			break;
		}
		current_block = current_block->next;
	}
	//if the end is reached, then the block should be added at the end
	if (i == list->size - 1)
		pos = list->size;

	return pos;
}

//function that reimplements the original malloc()
void MALLOC(doubly_linked_list_t **seg_list, doubly_linked_list_t **allocd_list,
			unsigned int nr_bytes)
{
	if (!(*allocd_list))
		*allocd_list = dll_create_list(nr_bytes);

	if (!nr_bytes)
		return;

	dll_block_t *curr;

	//searches for a free block of the specified size
	curr = dll_get_free_block(seg_list, nr_bytes);
	if (curr) {
		unsigned int pos;
		//if the returned block has a size bigger than wanted, it is fragmented
		//and the remaining block is added to the segregated free list
		if (((info *)curr->data)->data_size > nr_bytes) {
			header.n_fragments++;
			unsigned int i;
			i = ((info *)curr->data)->data_size - nr_bytes;
			seg_list[i]->data_size = i;
			if (seg_list[i]->size == 0)
				seg_list[i]->address_start = ((info *)curr->data)->address
											+ nr_bytes;
			//searches for the fragment's position to keep the sfl sorted
			pos = dll_get_pos(seg_list[i],
							  ((info *)curr->data)->address + nr_bytes);
			dll_add_nth_block_sfl(seg_list[i], pos,
								  ((info *)curr->data)->reconstruction_type,
								  ((info *)curr->data)->address + nr_bytes);
			header.n_free_blocks++;
			seg_list[i]->size++;
		}
		//adds the new block to the allocated-memory list
		pos = dll_get_pos(*allocd_list, ((info *)curr->data)->address);
		dll_add_nth_block_aml(*allocd_list, pos, NULL,
							  ((info *)curr->data)->reconstruction_type,
							  nr_bytes, ((info *)curr->data)->address);

		//the data from the removed block is freed
		(*allocd_list)->size++;
		((info *)curr->data)->data_size = 0;
		((info *)curr->data)->address = 0;
		if (((info *)curr->data)->values)
			free(((info *)curr->data)->values);
		free(curr->data);
		curr->data = NULL;
		free(curr);
		//adjusts the header info
		header.n_allocd_blocks++;
		header.n_free_blocks--;
		header.allocd_bytes += nr_bytes;
		header.malloc_calls++;
	} else {
		printf("Out of memory\n");
	}
}

//function that reimplements the original free()
void MYFREE(doubly_linked_list_t **seg_list, doubly_linked_list_t **allocd_list,
			unsigned long address)
{
	if (!allocd_list || !(*allocd_list)) {
		printf("Invalid free\n");
		return;
	}

	if (address == 0)
		return;

	dll_block_t *current_block;
	current_block = (*allocd_list)->head;
	int pos = -1;

	//checks if the beginning address is in an alloc'd block
	for (unsigned int i = 0; i < (*allocd_list)->size; i++) {
		if (((info *)current_block->data)->address == address) {
			pos = i;
			break;
		}
		current_block = current_block->next;
	}

	if (pos == -1) {
		printf("Invalid free\n");
		return;
	}

	//the block is then removed from the allocated-memory list
	current_block = dll_remove_nth_block(*allocd_list, pos);
	if ((*allocd_list)->size == 0)
		(*allocd_list)->head = NULL;

	unsigned int poz;
	unsigned int size = ((info *)(current_block->data))->data_size;
	header.allocd_bytes -= size;
	//into the segregated free list, a block of the corresponding size is added
	poz = dll_get_pos(seg_list[size], ((info *)current_block->data)->address);
	dll_add_nth_block_sfl(seg_list[size], poz,
						  ((info *)current_block->data)->reconstruction_type,
						  ((info *)current_block->data)->address);
	seg_list[size]->size++;
	//the data from the removed block is freed
	if (((info *)current_block->data)->values)
		free(((info *)current_block->data)->values);
	free((info *)current_block->data);
	free(current_block);
	header.n_free_blocks++;
	header.n_allocd_blocks--;
	header.free_calls++;
}

//function that dumps all the info related to the heap
//it is used in case of a seg fault or when called from the terminal
void DUMP_MEMORY(doubly_linked_list_t **seg_list,
				 doubly_linked_list_t *allocd_list)
{
	printf("+++++DUMP+++++\n");
	printf("Total memory: %d bytes\n", header.heap_size);
	printf("Total allocated memory: %d bytes\n", header.allocd_bytes);
	unsigned int free_mem = header.heap_size - header.allocd_bytes;
	printf("Total free memory: %d bytes\n", free_mem);
	printf("Free blocks: %d\n", header.n_free_blocks);
	printf("Number of allocated blocks: %d\n", header.n_allocd_blocks);
	printf("Number of malloc calls: %d\n", header.malloc_calls);
	printf("Number of fragmentations: %d\n", header.n_fragments);
	printf("Number of free calls: %d\n", header.free_calls);
	for (int i = 1; i <= (1 << (2 + header.num_lists)); i++) {
		if (seg_list[i]->head) {
			printf("Blocks with %d bytes - %d free block(s) :", i,
				   seg_list[i]->size);
			dll_print_block_address(seg_list[i]);
		}
	}

	printf("Allocated blocks :");
	if (header.malloc_calls != 0 && allocd_list->head) {
		dll_block_t *current_block = allocd_list->head;
		for (unsigned int i = 0; i < allocd_list->size; i++) {
			printf(" (0x%lx - %d)", ((info *)current_block->data)->address,
				   ((info *)current_block->data)->data_size);
			current_block = current_block->next;
		}
	}
	printf("\n");
	printf("-----DUMP-----\n");
}

//function that frees all the data associated to the heap
void DESTROY_HEAP(doubly_linked_list_t **seg_list,
				  doubly_linked_list_t *allocd_list, int num_lists)
{
	if (!seg_list) {
		DIE(!seg_list, "Initialize the list first");
		return;
	}
	//goes through each list and frees all the blocks and
	//the data stored in them
	int length = 1 << (2 + num_lists);
	for (int i = 0; i < length + 1; i++) {
		if (!seg_list[i])
			continue;
		if (!seg_list[i]->head) {
			free(seg_list[i]);
			continue;
		}
		dll_free(&seg_list[i]);
	}

	free(seg_list);
	seg_list = NULL;

	if (!allocd_list)
		return;

	dll_free(&allocd_list);
}

//function that checks if the given address is in an alloc'd block
unsigned int dll_check_address(doubly_linked_list_t *allocd_list,
							   unsigned long address)
{
	unsigned int pos = 0;
	dll_block_t *current_block;
	current_block = allocd_list->head;
	//checks if the address is between the beginning and end of the same block
	for (unsigned int i = 0; i < allocd_list->size; i++) {
		if (((info *)current_block->data)->address <= address &&
			((info *)current_block->data)->address +
			((info *)current_block->data)->data_size - 1 >= address) {
			pos = i + 1;
			break;
		}
		current_block = current_block->next;
	}

	return pos;
}

//checks if the entire space between two addresses is alloc'd
int dll_check_cont_space(doubly_linked_list_t *allocd_list,
						 unsigned int pos_beg, unsigned int pos_end)
{
	dll_block_t *current_block, *next_block;
	current_block = allocd_list->head;
	next_block = allocd_list->head->next;
	//moves through the list to the beginning position
	for (unsigned int i = 0; i < pos_beg; i++) {
		current_block = next_block;
		next_block = next_block->next;
	}

	int ok = 1;
	//the beginning of the next block should be
	//the current one's plus its size
	for (unsigned int i = pos_beg; i < pos_end; i++) {
		if (((info *)current_block->data)->address +
			((info *)current_block->data)->data_size !=
			((info *)next_block->data)->address) {
			ok = 0;
			break;
		}
		current_block = next_block;
		next_block = next_block->next;
	}

	return ok;
}

//function that reimplements the original write()
int WRITE(doubly_linked_list_t **seg_list, doubly_linked_list_t *allocd_list,
		  unsigned long address, char *data, int nr_bytes)
{
	if (!allocd_list || !allocd_list->head) {
		printf("Segmentation fault (core dumped)\n");
		DUMP_MEMORY(seg_list, allocd_list);
		DESTROY_HEAP(seg_list, allocd_list, header.num_lists);
		return 0;
	}

	int n = strlen(data);
	if (nr_bytes > n)
		nr_bytes = n;

	//checks if the beginning is in an alloc'd block
	unsigned int pos_beg, pos_end;
	pos_beg = dll_check_address(allocd_list, address);
	if (pos_beg == 0) {
		printf("Segmentation fault (core dumped)\n");
		DUMP_MEMORY(seg_list, allocd_list);
		DESTROY_HEAP(seg_list, allocd_list, header.num_lists);
		return 0;
	}
	//checks if the end is in an alloc'd block
	pos_end = dll_check_address(allocd_list, address + nr_bytes - 1);
	if (pos_end == 0) {
		printf("Segmentation fault (core dumped)\n");
		DUMP_MEMORY(seg_list, allocd_list);
		DESTROY_HEAP(seg_list, allocd_list, header.num_lists);
		return 0;
	}
	pos_beg--; pos_end--;

	//checks if all the blocks inbetween are alloc'd
	int ok = dll_check_cont_space(allocd_list, pos_beg, pos_end);
	if (ok == 0) {
		printf("Segmentation fault (core dumped)\n");
		DUMP_MEMORY(seg_list, allocd_list);
		DESTROY_HEAP(seg_list, allocd_list, header.num_lists);
		return 0;
	}

	//moves through the list until the wanted area is reached
	dll_block_t *current_block;
	current_block = allocd_list->head;
	for (unsigned int i = 0; i < pos_beg; i++)
		current_block = current_block->next;

	//each character from the data string is copied in its corresponding place
	unsigned int i;
	for (int index = 0; index < nr_bytes; index++) {
		if (!((info *)current_block->data)->values)
			((info *)current_block->data)->values =
			malloc(((info *)current_block->data)->data_size * sizeof(char));
		i = address + index - ((info *)current_block->data)->address;
		//moves to the next block when the max size is reached
		if (i >= ((info *)current_block->data)->data_size) {
			current_block = current_block->next;
			i = 0;
		}
		if (!((info *)current_block->data)->values)
			((info *)current_block->data)->values =
			malloc(((info *)current_block->data)->data_size * sizeof(char));
		((info *)current_block->data)->values[i] = data[index];
	}
	return 1;
}

//function that reimplements the original read()
int READ(doubly_linked_list_t **seg_list, doubly_linked_list_t *allocd_list,
		 unsigned long address, int nr_bytes)
{
	if (!allocd_list || !allocd_list->head) {
		printf("Segmentation fault (core dumped)\n");
		DUMP_MEMORY(seg_list, allocd_list);
		DESTROY_HEAP(seg_list, allocd_list, header.num_lists);
		return 0;
	}

	unsigned int pos_beg, pos_end;
	//checks if the beginning is in an alloc'd block
	pos_beg = dll_check_address(allocd_list, address);
	if (pos_beg == 0) {
		printf("Segmentation fault (core dumped)\n");
		DUMP_MEMORY(seg_list, allocd_list);
		DESTROY_HEAP(seg_list, allocd_list, header.num_lists);
		return 0;
	}
	//checks if the end is in an alloc'd block
	pos_end = dll_check_address(allocd_list, address + nr_bytes - 1);
	if (pos_end == 0) {
		printf("Segmentation fault (core dumped)\n");
		DUMP_MEMORY(seg_list, allocd_list);
		DESTROY_HEAP(seg_list, allocd_list, header.num_lists);
		return 0;
	}
	pos_beg--; pos_end--;

	//checks if all the blocks inbetween are alloc'd
	int ok = dll_check_cont_space(allocd_list, pos_beg, pos_end);
	if (ok == 0) {
		printf("Segmentation fault (core dumped)\n");
		DUMP_MEMORY(seg_list, allocd_list);
		DESTROY_HEAP(seg_list, allocd_list, header.num_lists);
		return 0;
	}
	//moves through the list until the wanted area is reached
	dll_block_t *current_block;
	current_block = allocd_list->head;
	for (unsigned int i = 0; i < pos_beg; i++)
		current_block = current_block->next;

	//prints each character from the wanted area
	unsigned int i;
	for (int index = 0; index < nr_bytes; index++) {
		i = address + index - ((info *)current_block->data)->address;
		//moves to the next block when the max size is reached
		if (i >= ((info *)current_block->data)->data_size) {
			current_block = current_block->next;
			i = 0;
		}
		printf("%c", ((info *)current_block->data)->values[i]);
	}
	printf("\n");
	return 1;
}

int main(void)
{
	doubly_linked_list_t **sfl, *aml;
	int num_lists, num_bytes_per_list, reconstruction_type, n;
	unsigned long start_heap;
	char line[600], command[16];
	sfl = NULL;
	aml = NULL;
	while (1) {
		fgets(line, sizeof(line), stdin);
		if (strncmp(line, "INIT_HEAP", 9) == 0) {
			n = sscanf(line, "%s%lx%d%d%d", command, &start_heap, &num_lists,
					   &num_bytes_per_list, &reconstruction_type);
			sfl = INIT_HEAP(start_heap, num_lists, num_bytes_per_list,
							reconstruction_type);
			n--;
		}
		if (strncmp(line, "MALLOC", 6) == 0) {
			int nr_bytes;
			n = sscanf(line, "%s%d", command, &nr_bytes);
			MALLOC(sfl, &aml, nr_bytes);
			n--;
		}
		if (strncmp(line, "FREE", 4) == 0) {
			unsigned long address;
			n = sscanf(line, "%s%lx", command, &address);
			MYFREE(sfl, &aml, address);
			n--;
		}
		if (strncmp(line, "READ", 4) == 0) {
			int nr_bytes;
			unsigned long address;
			n = sscanf(line, "%s%lx%d", command, &address, &nr_bytes);
			int ok = READ(sfl, aml, address, nr_bytes);
			n--;
			if (ok == 0)
				break;
		}
		if (strncmp(line, "WRITE", 5) == 0) {
			int nr_bytes;
			char data[500];
			unsigned long address;
			n = sscanf(line, "%9s %lx \"%500[^\"]\" %d", command,
					   &address, data, &nr_bytes);
			int ok = WRITE(sfl, aml, address, data, nr_bytes);
			n--;
			if (ok == 0)
				break;
		}
		if (strncmp(line, "DUMP_MEMORY", 11) == 0)
			DUMP_MEMORY(sfl, aml);

		if (strncmp(line, "DESTROY_HEAP", 12) == 0) {
			DESTROY_HEAP(sfl, aml, num_lists);
			break;
		}
	}
	return 0;
}
