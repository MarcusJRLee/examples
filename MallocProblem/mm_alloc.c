/*
 * mm_alloc.c
 *
 * Stub implementations of the mm_* routines.
 */

#include "mm_alloc.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

/* Marcus: Global Variables */
void *beginning_of_heap = NULL;
void *curr_brk = NULL;
void *top_ptr = NULL;
size_t header_size = sizeof(void *)*2 + sizeof(char) + sizeof(size_t); // int: 4 bytes; char: 1 byte; size_t: 8 bytes
/* Marcus: End; Global Variables */

/* Marcus: Helper Functions */
void prep_block(void *ptr) {
	size_t size = *((size_t *) (ptr + sizeof(void *)*2 + sizeof(char)));
	zero_bytes(ptr + header_size, size);
}

void zero_bytes(void *ptr, size_t size) {
	for (size_t i = 0; i < size; i++) {
		*((char *) (ptr + i)) = 0;
	}
}

void coalesce(void *ptr) {
	void *c_prev = *((void **) ptr); // *prev
	void *c_next = *((void **) (ptr + sizeof(void *))); // *next
	// char c_free = *((char *) (ptr + sizeof(void *)*2)); // free; should be 't'
	size_t c_size = *((size_t *) (ptr + sizeof(void *)*2 + sizeof(char))); // size

	if (c_next == NULL) {
		top_ptr = c_prev;
		// printf("would have been decrementing sbrk\n");
		// sbrk(-(header_size + c_size));
		curr_brk = sbrk(0);
		return;
	}

	// void *n_prev = *((void **) c_next); // *prev
	void *n_next = *((void **) (c_next + sizeof(void *))); // *next
	char n_free = *((char *) (c_next + sizeof(void *)*2)); // free; should be 't'
	size_t n_size = *((size_t *) (c_next + sizeof(void *)*2 + sizeof(char))); // size

	if (n_free == 't') {
		*((size_t *) (ptr + sizeof(void *)*2 + sizeof(char))) = c_size + header_size + n_size; // changing c_size
		*((void **) (ptr + sizeof(void *))) = n_next; // changing c_next
		coalesce(ptr);
		return;
	}

	if (c_prev != NULL) {
		// void *p_prev = *((void **) c_prev); // *prev
		// void *p_next = *((void **) (c_prev + sizeof(void *))); // *next
		char p_free = *((char *) (c_prev + sizeof(void *)*2)); // free; should be 't'
		size_t p_size = *((size_t *) (c_prev + sizeof(void *)*2 + sizeof(char))); // size

		if (p_free == 't') {
			*((size_t *) (c_prev + sizeof(void *)*2 + sizeof(char))) = p_size + header_size + c_size; // changing p_size
			*((void **) (c_prev + sizeof(void *))) = c_next; // changing p_next
			coalesce(c_prev);
			return;
		}
	}

	return;
}

void *malloc_helper(size_t size) {
	/* Marcus: Implementing Function */
	if (size == 0) {
		return NULL;
	}
	bool move_brk = 0;
	void *return_value = NULL;
	if (beginning_of_heap == NULL) {
		beginning_of_heap = sbrk(0);
		curr_brk = beginning_of_heap;
		move_brk = 1;
	} else if (beginning_of_heap == curr_brk) {
		move_brk = 1;
	}
	if (!move_brk) {
		// Iterate through already allocated memory.
		bool keep_going = 1;
		void *curr_ptr = beginning_of_heap;
		void *c_prev = *((void **) curr_ptr); // *prev
		void *c_next = *((void **) (curr_ptr + sizeof(void *))); // *next
		char c_free = *((char *) (curr_ptr + sizeof(void *)*2)); // free
		size_t c_size = *((size_t *) (curr_ptr + sizeof(void *)*2 + sizeof(char))); // size
		while (c_next != NULL || keep_going) {
			if (c_free == 't' && c_size >= size) {
				if (c_size >= size + header_size + 1) {
					// Make new block
					void *new_ptr = curr_ptr + header_size + size;
					printf("%d\n", header_size);
					*((void **) new_ptr) = curr_ptr; // *prev
					*((void **) (new_ptr + sizeof(void *))) = c_next; // *next
					*((char *) (new_ptr + sizeof(void *)*2)) = 't'; // free
					*((size_t *) (new_ptr + sizeof(void *)*2 + sizeof(char))) = c_size - size - header_size; // size

					// Change old block
					*((size_t *) (curr_ptr + sizeof(void *)*2 + sizeof(char))) = size; // changing c_size to size
					*((void **) (curr_ptr + sizeof(void *))) = new_ptr; // changing *next
					*((char *) (curr_ptr + sizeof(void *)*2)) = 'f'; // changing free

					return_value = curr_ptr + header_size;
					return return_value;
				} else {
					// Changing block
					*((char *) (curr_ptr + sizeof(void *)*2)) = 'f'; // changing free

					return_value = curr_ptr + header_size;
					return return_value;
				}
			} else {
				curr_ptr += header_size + c_size;
				c_prev = *((void **) curr_ptr);
				c_next = *((void **) (curr_ptr + sizeof(void *)));
				c_free = *((char *) (curr_ptr + sizeof(void *)*2));
				c_size = *((size_t *) (curr_ptr + sizeof(void *)*2 + sizeof(char)));
				if (c_next == NULL) keep_going = 0;
			}
		}
		move_brk = 1;
	}
	if (move_brk) {
		void *memory_ptr = sbrk(header_size + size); // Should be equal to curr_brk at this point
		if (memory_ptr == (void *) -1) {
			return NULL; // If memory could not be allocated.
		}
		curr_brk = sbrk(0); // Should be equal to memory_start + header_size + size

		// Make new block
		*((void **) memory_ptr) = top_ptr; // *prev, is a void *
		*((void **) (memory_ptr + sizeof(void *))) = NULL; // *next, is a void *
		*((char *) (memory_ptr + sizeof(void *)*2)) = 'f'; // free, is a char
		*((size_t *) (memory_ptr + sizeof(void *)*2 + sizeof(char))) = size; // size, is a size_t

		// Change old top block
		if (top_ptr != NULL) *((void **) (top_ptr + sizeof(void *))) = memory_ptr; // *next, is a void *

		return_value = memory_ptr + header_size;
		top_ptr = memory_ptr;
		return return_value;
	}
	// Execution should not reach here
	printf("Error in code exectution: mm_malloc\n");
	/* Marcus: End Implementing Function */
}
/* Marcus: End Helper Functions */

void *mm_malloc(size_t size) {
	/* Marcus: Implement Function */
	void *return_value = malloc_helper(size);
	if (return_value == NULL) return NULL;
	zero_bytes(return_value, size);
	return return_value;
	/* Marcus: End Implent Function */
    /* YOUR CODE HERE */
    // return NULL;
}

void *mm_realloc(void *ptr, size_t size) {

	if (size == 0) {
		mm_free(ptr);
		return NULL;
	} else if (ptr == NULL) {
		return mm_malloc(size);
	}

	size_t old_size = *((size_t *) (ptr - sizeof(size_t)));

	// Check to see if there is enough space
	void *check_value = malloc_helper(size);
	if (check_value == NULL) return NULL;
	mm_free(check_value);


	mm_free(ptr);
	void *return_value = malloc_helper(size);


	size_t mem_size = old_size;
	if (mem_size > size) mem_size = size;
	memcpy(return_value, ptr, mem_size);

	size_t d_size = size - old_size;

	if (size > old_size) {
		zero_bytes(return_value + old_size, d_size);
	}

	size_t new_size = *((size_t *) (return_value - sizeof(size_t)));

	if (size < new_size) {
		zero_bytes(return_value + size, new_size - size);
	}


	return return_value;

    /* YOUR CODE HERE */
    // return NULL;
}

void mm_free(void *ptr) {
	/* Marcus: Implementing Function */
	if (ptr == NULL) {
		return;
	}
	*((char *) (ptr - sizeof(size_t) - sizeof(char))) = 't'; // changing free to true
	coalesce(ptr - header_size);
	return;
	/* Marcus: End Implementing Function */
    /* YOUR CODE HERE */
}



















