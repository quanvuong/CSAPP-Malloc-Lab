/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "qhv200+vr697",
    /* First member's full name */
    "Vasily Rudchenko",
    /* First member's email address */
    "vr697@nyu.edu",
    /* Second member's full name (leave blank if none) */
    "Quan Vuong",
    /* Second member's email address (leave blank if none) */
    "qhv200@nyu.edu"
};

#define MAX_POWER 20
#define TAKEN 1
#define FREE 0

#define WORD_SIZE 4 /* bytes */
#define D_WORD_SIZE 8
#define CHUNK ((1<<12)/WORD_SIZE) /* extend heap by this amount (words) */
#define STATUS_BIT_SIZE 3 // bits
#define HDR_FTR_SIZE 2 // in words
#define HDR_SIZE 1 // in words
#define FTR_SIZE 1 // in words
#define PRED_FIELD_SIZE 1 // in words
#define EPILOG_SIZE 2 // in words

// Read and write a word at address p
#define GET_BYTE(p) (*(char *)(p))
#define GET_WORD(p) (*(unsigned int *)(p))
#define PUT_WORD(p, val) (*(char **)(p) = (val))

// Get a bit mask where the lowest size bit is set to 1
#define GET_MASK(size) ((1 << size) - 1)

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

// Pack a size and allocated bit into a BIT_word
#define PACK(size, status) ((size<<STATUS_BIT_SIZE) | (status))

/* Round up to even */
#define EVENIZE(x) ((x + 1) & ~1)

// Read the size and allocation bit from address p
#define GET_SIZE(p)  ((GET_WORD(p) & ~GET_MASK(STATUS_BIT_SIZE)) >> STATUS_BIT_SIZE)
#define GET_STATUS(p) (GET_WORD(p) & 0x1)

// Address of block's footer
// Take in a pointer that points to the header
#define FTRP(header_p) ((char **)(header_p) + GET_SIZE(header_p) + 1)

// Get total size of a block
// Size indicates the size of the free space in a block
// Total size = size + size_of_header + size_of_footer = size + D_WORD_SIZE
// p must point to a header
#define GET_TOTAL_SIZE(p) (GET_SIZE(p) + HDR_FTR_SIZE)

// Define this so later when we move to store the list in heap,
// we can just change this function
#define GET_FREE_LIST_PTR(i) (main_free_list[i])
#define SET_FREE_LIST_PTR(i, ptr) (main_free_list[i] = ptr)

// Set pred or succ for free blocks
#define SET_PTR(p, ptr) (*(char **)(p) = (char *)(ptr))

// Get pointer to the word containing the address of pred and succ for a free block
// ptr should point to the start of the header
#define GET_PTR_PRED_FIELD(ptr) ((char **)(ptr) + HDR_SIZE)
#define GET_PTR_SUCC_FIELD(ptr) ((char **)(ptr) + HDR_SIZE + PRED_FIELD_SIZE)

// Get the pointer that points to the succ of a free block
// ptr should point to the header of the free block
#define GET_PRED(bp) (*(GET_PTR_PRED_FIELD(bp)))
#define GET_SUCC(bp) (*(GET_PTR_SUCC_FIELD(bp)))

// Given pointer to current block, return pointer to header of previous block
#define PREV_BLOCK_IN_HEAP(header_p) ((char **)(header_p) - GET_TOTAL_SIZE((char **)(header_p) - FTR_SIZE))

// Given pointer to current block, return pointer to header of next block
#define NEXT_BLOCK_IN_HEAP(header_p) (FTRP(header_p) + FTR_SIZE)

// Global variables
static char *main_free_list[MAX_POWER + 1];
static char **heap_ptr;

// Function Declarations
static size_t find_free_list_index(size_t words);

static void *extend_heap(size_t words);

static void *coalesce(void *bp);
static void *find_free_block(size_t words);
static void alloc_free_block(void *bp, size_t words);
static void place_block_into_free_list(char **bp);
static void remove_block_from_free_list(char **bp);

int mm_check();


/*
	Find the index of the free list which given size belongs to.
	Returns index.
	Index can be from 0 to MAX_POWER.
*/
static size_t find_free_list_index(size_t words) {
    int index = 0;

    while ((index <= MAX_POWER) && (words > 1))
    {
        words >>= 1;
        index++;
    }

    return index;
}

/*
	The function combines the current block in
	physical memory with neigboring free blocks.
	Returns the pointer to the beginning of this
	new free block.
	Coalesce is only called on a block that is not in the free list.
	As such, coalesce does not set pointer values.
*/
static void *coalesce(void *bp) {
	char **prev_block = PREV_BLOCK_IN_HEAP(bp);
	char **next_block = NEXT_BLOCK_IN_HEAP(bp);
	size_t prev_status = GET_STATUS(prev_block);
	size_t next_status = GET_STATUS(next_block);
	size_t new_size = GET_SIZE(bp);

	if (prev_status == TAKEN && next_status == TAKEN) {
		return bp;
	} else if (prev_status == TAKEN && next_status == FREE) {
		remove_block_from_free_list(next_block);
		new_size += GET_TOTAL_SIZE(next_block);

		PUT_WORD(bp, PACK(new_size, FREE));
		PUT_WORD(FTRP(next_block), PACK(new_size, FREE));
	} else if (prev_status == FREE && next_status == TAKEN) {
		remove_block_from_free_list(prev_block);
		new_size += GET_TOTAL_SIZE(prev_block);

		PUT_WORD(prev_block, PACK(new_size, FREE));
		PUT_WORD(FTRP(bp), PACK(new_size, FREE));
		bp = prev_block;
	} else {
		remove_block_from_free_list(prev_block);
		remove_block_from_free_list(next_block);
		new_size += GET_TOTAL_SIZE(prev_block) + GET_TOTAL_SIZE(next_block);

		PUT_WORD(prev_block, PACK(new_size, FREE));
		PUT_WORD(FTRP(next_block), PACK(new_size, FREE));
		bp = prev_block;
	}

	return bp;
}

/*
	Relies on mem_sbrk to create a new free block.
	Does not coalesce.
	Does not place into free list.
	Returns pointer to the new block of memory with
	header and footer already defined.
	Returns NULL if we ran out of physical memory.
*/
static void *extend_heap(size_t words) {
	char **bp; // pointer to the free block formed by extending memory
	char **end_pointer; // pointer to the end of the free block
	size_t words_extend = EVENIZE(words); // make sure double aligned
	size_t words_extend_tot = words_extend + HDR_FTR_SIZE; // add header and footer

	// extend memory by so many words
	// multiply words by WORD_SIZE because mem_sbrk takes input as bytes
	if ((long)(bp = mem_sbrk((words_extend_tot) * WORD_SIZE)) == -1) {
		return NULL;
	}

	// offset to make use of old epilog and add space for new epilog
	bp -= EPILOG_SIZE;

	// set new block header/footer to size (in words)
	PUT_WORD(bp, PACK(words_extend, FREE));
	PUT_WORD(FTRP(bp), PACK(words_extend, FREE));

	// add epilog to the end
	end_pointer = bp + words_extend_tot;
	PUT_WORD(end_pointer, PACK(0, TAKEN));
	PUT_WORD(FTRP(end_pointer), PACK(0, TAKEN));

	return bp;
}

/*
	Finds the block from the main_free_list that is large
	enough to hold the amount of words specified.
	Returns the pointer to that block.
	Does not take the block out of the free list.
	Does not extend heap.
	Returns the pointer to the block.
	Returns NULL if block large enough is not found.
*/
static void *find_free_block(size_t words) {
	char **bp;
	size_t index = find_free_list_index(words);

	// check if first free list can contain large enough block
	if ((bp = GET_FREE_LIST_PTR(index)) != NULL && GET_SIZE(bp) >= words) {
		// iterate through blocks
		while(1) {
			// if block is of exact size, return right away
			if (GET_SIZE(bp) == words) {
				return bp;
			}

			// if next block is not possible, return current one
			if (GET_SUCC(bp) == NULL || GET_SIZE(GET_SUCC(bp)) < words) {
				return bp;
			} else {
				bp = GET_SUCC(bp);
			}
		}
	}

	// move on from current free list
	index++;

	// find a large enough non-empty free list
	while (GET_FREE_LIST_PTR(index) == NULL && index < MAX_POWER) {
		index++;
	}

	// if there is a non-NULL free list, go until the smallest block in free list
	if ((bp = GET_FREE_LIST_PTR(index)) != NULL) {
		while (GET_SUCC(bp) != NULL) {
			bp = GET_SUCC(bp);
		}

		return bp;
	} else { // if no large enough free list available, return NULL
		return NULL;
	}
}

/*
	The function takes free block and changes status to taken.
	The free block is assumed to have been removed from free list.
	The function reduces the size of the free block (splits it) if size is too large.
	Too large is a free block whose size is > needed size + HDR_FTR_SIZE
	The remaining size is either placed in free_list or left hanging if it is > 0.
	If remaining size is 0 it becomes part of the allocated block.
	bp input is the block that you found already that is large enough.
	Assume that size in words given is <= the size of the block at input.
*/
static void alloc_free_block(void *bp, size_t words) {
	size_t bp_size = GET_SIZE(bp);
	size_t bp_tot_size = bp_size + HDR_FTR_SIZE;

	size_t needed_size = words;
	size_t needed_tot_size = words + HDR_FTR_SIZE;

	int new_block_tot_size = bp_tot_size - needed_tot_size;
	int new_block_size = new_block_tot_size - HDR_FTR_SIZE;

	// the block created from extra free space
	char **new_block;

	// if size of block is larger than needed size, split the block
	// handle new block by making it part of the free block ecosystem
	if ((int)new_block_size > 0) {
		// set new block pointer at offset from start of bp
		new_block = (char **)(bp) + needed_tot_size;

		// set new block's size and status
		PUT_WORD(new_block, PACK(new_block_size, FREE));
		PUT_WORD(FTRP(new_block), PACK(new_block_size, FREE));

		// set bp size to exact needed size
		PUT_WORD(bp, PACK(needed_size, TAKEN));
		PUT_WORD(FTRP(bp), PACK(needed_size, TAKEN));

		// check if new block can become larger than it is
		coalesce(new_block);

		// handle this new block by putting back into free list
		place_block_into_free_list(new_block);
	} else if (new_block_size == 0) {
		// if the new_block_size is zero there is no point in separating the blocks
		// thus the extra two words are just kept as part of the allocated block
		needed_size += HDR_FTR_SIZE;

		PUT_WORD(bp, PACK(needed_size, TAKEN));
		PUT_WORD(FTRP(bp), PACK(needed_size, TAKEN));
	} else {
		// if exact size just change status
		PUT_WORD(bp, PACK(needed_size, TAKEN));
		PUT_WORD(FTRP(bp), PACK(needed_size, TAKEN));
	}
}

/*
	Removes a block from the free list if block size is larger than zero.
	Does nothing if it is zero.
	Does not return the pointer to that block.
*/
static void remove_block_from_free_list(char **bp) {
	char **prev_block = GET_PRED(bp);
	char **next_block = GET_SUCC(bp);
	int index;

	if (GET_SIZE(bp) == 0) {
		return;
	}

	// if largest block in free list set free list to next ptr
	if (prev_block == NULL) {
		index = find_free_list_index(GET_SIZE(bp));
		GET_FREE_LIST_PTR(index) = next_block;
	} else { // if not largest block update pointer for prev block to next ptr
		SET_PTR(GET_PTR_SUCC_FIELD(prev_block), next_block);
	}

	// next_block is not NULL, update the block to point to prev block
	if (next_block != NULL) {
		SET_PTR(GET_PTR_PRED_FIELD(next_block), prev_block);
	}

	// clear current block's pointers
	SET_PTR(GET_PTR_PRED_FIELD(bp), NULL);
	SET_PTR(GET_PTR_SUCC_FIELD(bp), NULL);
}

/*
	Places the block into the free list based on block size.
*/
static void place_block_into_free_list(char **bp) {
    size_t size = GET_SIZE(bp);
    int index = find_free_list_index(size);

    char **front_ptr = GET_FREE_LIST_PTR(index);
    char **back_ptr = NULL;

		// If the block size is zero than it doesn't belong in the free list
		// because it doesn't have enough space for pointers
		if (size == 0) {
			return;
		}

    // If the free list is empty
    if (front_ptr == NULL)
    {
        SET_PTR(GET_PTR_SUCC_FIELD(bp), NULL);
				SET_PTR(GET_PTR_PRED_FIELD(bp), NULL);
        SET_FREE_LIST_PTR(index, bp);
        return;
    }

    // If the new block is the biggest in the respective free list
    if (size >= GET_SIZE(front_ptr))
    {
				SET_FREE_LIST_PTR(index, bp);
        SET_PTR(GET_PTR_SUCC_FIELD(bp), front_ptr);
				SET_PTR(GET_PTR_PRED_FIELD(bp), NULL);
        return;
    }

    // Keep each free list sorted in descending order of size
    while (front_ptr != NULL && GET_SIZE(front_ptr) > size)
    {
        back_ptr = front_ptr;
        front_ptr = GET_SUCC(front_ptr);
    }

    // Reached the end of the free list
    if (front_ptr == NULL)
    {
        SET_PTR(GET_PTR_SUCC_FIELD(back_ptr), bp);
				SET_PTR(GET_PTR_PRED_FIELD(bp), back_ptr);
        SET_PTR(GET_PTR_SUCC_FIELD(bp), NULL);
        return;
    }
    else
    { // Haven't reached the end of the free list
        SET_PTR(GET_PTR_SUCC_FIELD(back_ptr), bp);
				SET_PTR(GET_PTR_PRED_FIELD(bp), back_ptr);
        SET_PTR(GET_PTR_SUCC_FIELD(bp), front_ptr);
				SET_PTR(GET_PTR_PRED_FIELD(front_ptr), bp);
        return;
    }

}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // Initialize the free list
    for (int i = 0; i <= MAX_POWER; i++) {
	    SET_FREE_LIST_PTR(i, NULL);
    }

    if ((long)(heap_ptr = mem_sbrk(4*WORD_SIZE)) == -1) // 2 for prolog, 2 for epilog
        return -1;

    PUT_WORD(heap_ptr, PACK(0, TAKEN)); // Prolog header
    PUT_WORD(FTRP(heap_ptr), PACK(0, TAKEN)); // Prolog footer

		char ** epilog = NEXT_BLOCK_IN_HEAP(heap_ptr);
    PUT_WORD(epilog, PACK(0, TAKEN)); // Epilog header
    PUT_WORD(FTRP(epilog), PACK(0, TAKEN)); // Epilog footer

		heap_ptr += HDR_FTR_SIZE; // Move past prolog

    if (extend_heap(CHUNK) == NULL)
        return -1;

    return 0;
}

/*
	Input is in bytes
*/

void *mm_malloc(size_t size)
{
	size_t words = ALIGN(size) / WORD_SIZE;
	size_t extend_size;
	char **bp;

	if (size == 0) {
		return NULL;
	}

	// check if there is a block that is large enough
	// if not, extend the heap
	if ((bp = find_free_block(words)) == NULL) {
		extend_size = words > CHUNK ? words : CHUNK;

		if ((bp = extend_heap(extend_size)) == NULL) {
			return NULL;
		}

		// do not remove block from free list because it is not in it
		alloc_free_block(bp, words);

		return bp;
	}

	remove_block_from_free_list(bp);
	alloc_free_block(bp, words);

	return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 * Role:
    - change the status of block to free
    - coalesce the block
    - place block into free_lists

 * Assume: ptr points to the beginning of a block header
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(ptr);

    PUT_WORD(ptr, PACK(size, FREE));
    PUT_WORD(FTRP(ptr), PACK(size, FREE));

    coalesce(ptr);

    place_block_into_free_list(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - 4);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

static void test_find_free_list_index()
{
    printf("Testing test_find_free_list_index.\n");

    int index_0 = find_free_list_index(0);
    assert(index_0 == 0);

    index_0 = find_free_list_index(1);
    assert(index_0 == 0);

    int index_1 = find_free_list_index(2);
    assert(index_1 == 1);

    index_1 = find_free_list_index(3);
    assert(index_1 == 1);

    int index_2 = find_free_list_index(4);
    assert(index_2 == 2);

    index_2 = find_free_list_index(5);
    assert(index_2 == 2);

    int index_3 = find_free_list_index(15);
    assert(index_3 == 3);

    int index_9 = find_free_list_index(1023);
    assert(index_9 == 9);

    int index_10 = find_free_list_index(1024);
    assert(index_10 == 10);

    int index_11 = find_free_list_index(2048);
    assert(index_11 == 11);

    int index_20 = find_free_list_index(1048600);
    assert(index_20 == 20);

    printf("Test passed.\n\n");
}

static void test_ALIGN()
{
    printf("Test ALIGN.\n");
    assert(0 == ALIGN(0));
    assert(8 == ALIGN(1));
    assert(8 == ALIGN(8));
    assert(104 == ALIGN(100));
    printf("Test passed.\n\n");
};

static void test_EVENIZE()
{
    printf("Test EVENIZE.\n");
    assert(4 == EVENIZE(3));
    assert(4 == EVENIZE(4));
    assert(0 == EVENIZE(0));
    assert(1002 == EVENIZE(1001));
    printf("Test passed.\n\n");
}

static void test_GET_SIZE()
{
    printf("Test GET_SIZE.\n");

    size_t size_8 = 8 << STATUS_BIT_SIZE;
    assert(8 == GET_SIZE(&size_8));

    size_t size_0 = 0 << STATUS_BIT_SIZE;
    assert(0 == GET_SIZE(&size_0));

    size_t size_1 = 1 << STATUS_BIT_SIZE;
    assert(1 == GET_SIZE(&size_1));

    printf("Test passed.\n\n");
}

static void test_GET_STATUS()
{
    printf("Test GET_STATUS.\n");

    size_t taken = 1;
    size_t free = 0;

    assert(GET_STATUS(&taken) == TAKEN);
    assert(GET_STATUS(&free) == FREE);

    printf("Test passed.\n\n");
}

static void test_PACK()
{
    // size >= 8
    printf("Test PACK.\n");
    int size_8_free = PACK(8, FREE);
    assert(GET_SIZE(&size_8_free) == 8);
    assert(GET_STATUS(&size_8_free) == FREE);

    int size_9_taken = PACK(9, TAKEN);
    assert(GET_SIZE(&size_9_taken) == 9);
    assert(GET_STATUS(&size_9_taken) == TAKEN);

    printf("Test passed.\n\n");
}

static void test_PUT_WORD()
{
    printf("Test PUT_WORD.\n");
    char ** p = malloc(WORD_SIZE);
    int test_value_1 = 0;
    int test_value_2 = 100;
    int test_value_3 = 1 << 5;

    PUT_WORD(p, test_value_1);
    assert((*p) == test_value_1);

    PUT_WORD(p, test_value_2);
    assert((*p) == test_value_2);

    PUT_WORD(p, test_value_3);
    assert((*p) == test_value_3);
    printf("Test passed.\n\n");
}

static void test_FTRP()
{
    printf("Test FTRP.\n");
    int test_block_size = 10; // Does not include size of header and footer
    char **header_p;
    char **ptr = malloc(WORD_SIZE*(test_block_size + HDR_FTR_SIZE));
    header_p = ptr;

    PUT_WORD(header_p, PACK(test_block_size, TAKEN));
    ptr += test_block_size + 1; // + 1 to move pass header

    assert(FTRP(header_p) == ptr);
    printf("Test passed.\n\n");
}

static void test_MAIN_FREE_LIST_INIT()
{
    printf("Test MAIN_FREE_LIST_INIT.\n");
    for (int i = 0; i <= MAX_POWER; i++)
        assert(GET_FREE_LIST_PTR(i) == NULL);
    printf("Test passed.\n\n");
}


static void test_SET_PTR()
{
    printf("Test SET_PTR.\n");
    char *ptr = malloc(sizeof(char *));
    char **p = malloc(sizeof(char **));

    SET_PTR(p, ptr);

    assert((*p) = ptr);
    printf("Test passed.\n\n");
    free(ptr);
    free(p);
}

static void test_GET_PTR_PRED_FIELD()
{
    printf("Test GET_PTR_PRED_FIELD.\n");
    char **bp = malloc(2*WORD_SIZE); // enough for hdr + pred field
    char **ptr_to_pred_field = bp + HDR_SIZE; // ptr_to_pred_field should point to the PRED field after this line

    assert(GET_PTR_PRED_FIELD(bp) == ptr_to_pred_field);
    printf("Test passed.\n\n");
    free(bp);
}

static void test_GET_PTR_SUCC_FIELD()
{
    printf("Test GET_PTR_SUCC_FIELD.\n");
    char **bp = malloc(3*WORD_SIZE); // enough for hdr + pred field + succ field
    char **ptr_to_succ_field = bp + HDR_SIZE + PRED_FIELD_SIZE; // ptr_to_pred_field should point to the succ field after this line

    assert(GET_PTR_SUCC_FIELD(bp) == ptr_to_succ_field);
    printf("Test passed.\n\n");
    free(bp);
}


static void test_GET_SUCC()
{
    printf("Test test_GET_SUCC.\n");
    char **bp = malloc(3*WORD_SIZE); // enough for hdr + pred field + succ field
    char **ptr_to_succ_field = bp + HDR_SIZE + PRED_FIELD_SIZE; // ptr_to_pred_field should point to the succ field after this line

    assert(GET_SUCC(bp) == (*ptr_to_succ_field));
    printf("Test passed.\n\n");
    free(bp);
}

static void test_GET_PRED()
{
    printf("Test test_GET_PRED.\n");
    char **bp = malloc(2*WORD_SIZE); // enough for hdr + pred field
    char **ptr_to_pred_field = bp + HDR_SIZE; // ptr_to_pred_field should point to the pred field after this line

    assert(GET_PRED(bp) == (*ptr_to_pred_field));
    printf("Test passed.\n\n");
    free(bp);
}

// Initialize main_free_list to all NULL
// Insert 3 block of size 60, 52, 40 into the free list
static void initialize_free_lists_for_test()
{
    for (int i = 0; i <= MAX_POWER; i++) {
				SET_FREE_LIST_PTR(i, NULL);
    }

    int block_1_size = 60;
    int block_2_size = 53;
    int block_3_size = 40;
    int index = find_free_list_index(block_1_size);

    // Building the list
    char **block_1 = malloc((block_1_size+HDR_FTR_SIZE)*WORD_SIZE);
    PUT_WORD(block_1, PACK(block_1_size, FREE));
    PUT_WORD(FTRP(block_1), PACK(block_1_size, FREE));
		SET_FREE_LIST_PTR(index, block_1);
		SET_PTR(GET_PTR_PRED_FIELD(block_1), NULL);

    char **block_2 = malloc((block_2_size+HDR_FTR_SIZE)*WORD_SIZE);
    PUT_WORD(block_2, PACK(block_2_size, FREE));
    PUT_WORD(FTRP(block_2), PACK(block_2_size, FREE));
    SET_PTR(GET_PTR_SUCC_FIELD(block_1), block_2);
		SET_PTR(GET_PTR_PRED_FIELD(block_2), block_1);

    char **block_3 = malloc((block_3_size+HDR_FTR_SIZE)*WORD_SIZE);
    PUT_WORD(block_3, PACK(block_3_size, FREE));
    PUT_WORD(FTRP(block_3), PACK(block_3_size, FREE));
    SET_PTR(GET_PTR_SUCC_FIELD(block_2), block_3);
		SET_PTR(GET_PTR_PRED_FIELD(block_3), block_2);
    SET_PTR(GET_PTR_SUCC_FIELD(block_3), NULL);
}

// Case 2: Free list not empty. New block is the smallest block in list.
static void test_place_block_into_free_list_case_2()
{
    initialize_free_lists_for_test();

    // New block
    int new_b_size = 35;
    char **new_block = malloc((new_b_size+HDR_FTR_SIZE)*WORD_SIZE);
    PUT_WORD(new_block, PACK(new_b_size, FREE));
    PUT_WORD(FTRP(new_block), PACK(new_b_size, FREE));

    int index = find_free_list_index(new_b_size);

    char **end_of_list = GET_FREE_LIST_PTR(index);

    while (GET_SUCC(end_of_list) != NULL) {
        end_of_list = GET_SUCC(end_of_list);
    }

    place_block_into_free_list(new_block);

    assert(GET_SUCC(end_of_list) == new_block);
		assert(GET_PRED(new_block) == end_of_list);
    assert(GET_SUCC(new_block) == NULL);
    free(new_block);
}

// Case 3: free list is not empty. New block size is neither the largest or smallest in list.
static void test_place_block_into_free_list_case_3()
{
    initialize_free_lists_for_test();

    // New block
    int new_b_size = 50;
    char **new_block = malloc((new_b_size+HDR_FTR_SIZE)*WORD_SIZE);
    PUT_WORD(new_block, PACK(new_b_size, FREE));
    PUT_WORD(FTRP(new_block), PACK(new_b_size, FREE));

    int index = find_free_list_index(new_b_size);

    char **front_ptr = GET_FREE_LIST_PTR(index);
    char **back_ptr = NULL;

    while (GET_SIZE(front_ptr) > new_b_size) {
        back_ptr = front_ptr;
        front_ptr = GET_SUCC(front_ptr);
    }

    place_block_into_free_list(new_block);

    assert(GET_SUCC(back_ptr) == new_block);
		assert(GET_PRED(new_block) == back_ptr);
    assert(GET_SUCC(new_block) == front_ptr);
		assert(GET_PRED(front_ptr) == new_block);
    free(new_block);
}


// Case 4: Free list not empty. New block is the biggest block in list
static void test_place_block_into_free_list_case_4()
{
    initialize_free_lists_for_test();

    // New Block
    int new_b_size = 60;
    char **new_block = malloc((new_b_size+HDR_FTR_SIZE)*WORD_SIZE);
    PUT_WORD(new_block, PACK(new_b_size, FREE));
    PUT_WORD(FTRP(new_block), PACK(new_b_size, FREE));

    int index = find_free_list_index(new_b_size);
    char **previous_biggest = GET_FREE_LIST_PTR(index);

    place_block_into_free_list(new_block);

    assert(GET_FREE_LIST_PTR(index) == new_block);
    assert(GET_SUCC(new_block) == previous_biggest);
		assert(GET_PRED(new_block) == NULL);
    free(new_block);
}

// Case 5: New block is of size zero
static void test_place_block_into_free_list_case_5()
{
    initialize_free_lists_for_test();

    // New Block
    int new_b_size = 0;
    char **new_block = malloc((new_b_size+HDR_FTR_SIZE)*WORD_SIZE);
    PUT_WORD(new_block, PACK(new_b_size, FREE));
    PUT_WORD(FTRP(new_block), PACK(new_b_size, FREE));

    int index = find_free_list_index(new_b_size);

    place_block_into_free_list(new_block);

    assert(GET_FREE_LIST_PTR(index) == NULL);
    assert(GET_SIZE(new_block) == 0);
		assert(GET_STATUS(new_block) == FREE);
    free(new_block);
}


static void test_place_block_into_free_list()
{
    printf("Test place_block_into_free_list.\n");

    char *free_list_backup[MAX_POWER + 1]; // backing up main_free_list to restore after test
    for (int i = 0; i <= MAX_POWER; i++) {
        free_list_backup[i] = GET_FREE_LIST_PTR(i);
    }

    // Case 1: Free list currently empty
    for (int i = 0; i <= MAX_POWER; i++) {
				SET_FREE_LIST_PTR(i, NULL);
    }

    int new_block_size = 20; // words
    char **bp = malloc((new_block_size+HDR_FTR_SIZE)*WORD_SIZE);
    PUT_WORD(bp, PACK(new_block_size, FREE));
    PUT_WORD(FTRP(bp), PACK(new_block_size, FREE));

    place_block_into_free_list(bp);

    int list = find_free_list_index(GET_SIZE(bp));
    assert(GET_FREE_LIST_PTR(list) == bp);
    assert(GET_SUCC(bp) == NULL);
		assert(GET_PRED(bp) == NULL);
    free(bp);

    // Case 2: Free list not empty. New block is the smallest block in list.
    test_place_block_into_free_list_case_2();

    // Case 3: free list is not empty. New block size is neither the largest or smallest in list.
    test_place_block_into_free_list_case_3();

    // Case 4: Free list not empty. New block is the biggest block in list
    test_place_block_into_free_list_case_4();

		// Case 5: New block is of size zero
		test_place_block_into_free_list_case_5();

    // Restoring the value in main_free_list so that test doesn't have harmful side effects
    for (int i = 0; i <= MAX_POWER; i++) {
			SET_FREE_LIST_PTR(i, free_list_backup[i]);
    }

    printf("Test passed.\n\n");
}

static void test_remove_block_from_free_list() {
	printf("Test remove_block_from_free_list.\n");

	char *free_list_backup[MAX_POWER + 1]; // backing up main_free_list to restore after test
	for (int i = 0; i <= MAX_POWER; i++) {
			free_list_backup[i] = GET_FREE_LIST_PTR(i);
	}

	initialize_free_lists_for_test();

	int index = find_free_list_index(60); // arbitrary size to get index
	char **first_block = GET_FREE_LIST_PTR(index);
	char **second_block = GET_SUCC(first_block);
	char **third_block = GET_SUCC(second_block);

	// Case 1: remove a block from between two other blocks
	remove_block_from_free_list(second_block);

	assert(GET_SUCC(first_block) == third_block);
	assert(GET_PRED(third_block) == first_block);
	assert(GET_SUCC(second_block) == NULL);
	assert(GET_PRED(second_block) == NULL);

	// Case 2: remove block from the beginning of the free list
	remove_block_from_free_list(first_block);

	assert(GET_FREE_LIST_PTR(index) == third_block);
	assert(GET_PRED(third_block) == NULL);
	assert(GET_SUCC(first_block) == NULL);
	assert(GET_PRED(first_block) == NULL);

	// Case 3: remove last block
	remove_block_from_free_list(third_block);

	assert(GET_FREE_LIST_PTR(index) == NULL);
	assert(GET_SUCC(third_block) == NULL);
	assert(GET_PRED(third_block) == NULL);

	// Case 4: try to remove zero-size free block
	char **size_zero_block = malloc(WORD_SIZE*HDR_FTR_SIZE);
	PUT_WORD(size_zero_block, PACK(0, FREE));
	PUT_WORD(FTRP(size_zero_block), PACK(0, FREE));

	remove_block_from_free_list(size_zero_block);

	assert(GET_SIZE(size_zero_block) == 0);
	assert(GET_STATUS(size_zero_block) == FREE);
	assert(GET_SIZE(FTRP(size_zero_block)) == 0);
	assert(GET_STATUS(FTRP(size_zero_block)) == FREE);

  printf("Test passed.\n\n");
}

static void test_extend_heap() {
	printf("Test extend_heap.\n");

	char **test_heap_ptr;
	char **initial_epilog_location;
	char **initial_heap_end;
	char **new_epilog_location;
	size_t initial_heap_size = 8; // words
	size_t new_block_size = 50; // words

	// create initial heap and epilog taken space at the end
	test_heap_ptr = mem_sbrk((initial_heap_size) * WORD_SIZE);
	initial_epilog_location = test_heap_ptr + initial_heap_size - EPILOG_SIZE;
	initial_heap_end = test_heap_ptr + initial_heap_size;
	new_epilog_location = initial_heap_end + new_block_size + HDR_FTR_SIZE - EPILOG_SIZE;
	PUT_WORD(initial_epilog_location, PACK(0, TAKEN));
	PUT_WORD(initial_epilog_location + HDR_SIZE, PACK(0, TAKEN));

	extend_heap(50);

	// check that block was created and placed to start where old heap epilog was
	assert(GET_SIZE(initial_epilog_location) == new_block_size);
	assert(GET_STATUS(initial_epilog_location) == FREE);
	assert(GET_SIZE(FTRP(initial_epilog_location)) == new_block_size);
	assert(GET_STATUS(FTRP(initial_epilog_location)) == FREE);

	// check that extra epilog was created at the end of the new heap space
	assert(GET_SIZE(new_epilog_location) == 0);
	assert(GET_STATUS(new_epilog_location) == TAKEN);
	assert(GET_SIZE(FTRP(new_epilog_location)) == 0);
	assert(GET_STATUS(FTRP(new_epilog_location)) == TAKEN);

  printf("Test passed.\n\n");
}

static void test_find_free_block() {
	printf("Test find_free_block.\n");

	char *free_list_backup[MAX_POWER + 1]; // backing up main_free_list to restore after test
	for (int i = 0; i <= MAX_POWER; i++) {
		free_list_backup[i] = GET_FREE_LIST_PTR(i);
	}

	char **bp;

	initialize_free_lists_for_test();

	// Case 1: find by size that is smaller than index
	bp = find_free_block(2);
	assert(GET_SIZE(bp) == 40);
	printf("Case 1 passed.\n");

	// Case 2: find by size that matches smallest block in index
	bp = find_free_block(33);
	assert(GET_SIZE(bp) == 40);
	printf("Case 2 passed.\n");

	// Case 3: find by size so that middle block in index is found
	bp = find_free_block(42);
	assert(GET_SIZE(bp) == 53);
	printf("Case 3 passed.\n");

	// Case 4: find by size so that largest block in index is used
	bp = find_free_block(59);
	assert(GET_SIZE(bp) == 60);
	printf("Case 4 passed.\n");

	// Case 5: find by size that is too big for current index and other indices
	bp = find_free_block(63);
	assert(bp == NULL);
	printf("Case 5 passed.\n");

	// Case 6: find by size that is too big from the start
	bp = find_free_block(120);
	assert(bp == NULL);
	printf("Case 6 passed.\n");

	// Case 7: find by exact size
	bp = find_free_block(53);
	assert(GET_SIZE(bp) == 53);
	printf("Case 7 passed.\n");

	// Case 8: find zero size - should still attempt to get the next-largest block
	bp = find_free_block(0);
	assert(GET_SIZE(bp) == 40);
	printf("Case 8 passed.\n");

	// Restoring the value in main_free_list so that test doesn't have harmful side effects
	for (int i = 0; i <= MAX_POWER; i++) {
		SET_FREE_LIST_PTR(i, free_list_backup[i]);
	}

	printf("Test passed.\n\n");
}

static void test_PREV_IN_HEAP() {
	printf("Testing PREV_IN_HEAP.\n");

	size_t prev_size = 4;
	size_t curr_size = 10;
	size_t total_needed_test_size = prev_size + curr_size + HDR_FTR_SIZE*2;

	char **ptr = malloc(WORD_SIZE * total_needed_test_size);
	char **p_prev = ptr;
	char **p_curr = ptr + HDR_FTR_SIZE + prev_size;

	PUT_WORD(p_prev, PACK(prev_size, FREE));
	PUT_WORD(FTRP(p_prev), PACK(prev_size, FREE));

	PUT_WORD(p_curr, PACK(curr_size, FREE));
	PUT_WORD(FTRP(p_curr), PACK(curr_size, FREE));

	char **tmp = ((char **)(p_curr) - GET_SIZE((char **)(p_curr) - 1) - HDR_FTR_SIZE);

	assert(p_prev == PREV_BLOCK_IN_HEAP(p_curr));

	printf("PREV test passed.\n\n");
}

static void test_NEXT_IN_HEAP() {
	printf("Testing NEXT_IN_HEAP.\n");

	size_t prev_size = 4;
	size_t curr_size = 10;
	size_t total_needed_test_size = prev_size + curr_size + HDR_FTR_SIZE*2;

	char **ptr = malloc(WORD_SIZE * total_needed_test_size);
	char **p_prev = ptr;
	char **p_curr = ptr + HDR_FTR_SIZE + prev_size;

	PUT_WORD(p_prev, PACK(prev_size, FREE));
	PUT_WORD(FTRP(p_prev), PACK(prev_size, FREE));

	PUT_WORD(p_curr, PACK(curr_size, FREE));
	PUT_WORD(FTRP(p_curr), PACK(curr_size, FREE));

	char **tmp = ((char **)(p_curr) - GET_SIZE((char **)(p_curr) - 1) - HDR_FTR_SIZE);

	assert(p_curr == NEXT_BLOCK_IN_HEAP(p_prev));

	printf("NEXT test passed.\n\n");
}

static void test_coalesce() {
	printf("Testing coalesce.\n");

	char *free_list_backup[MAX_POWER + 1]; // backing up main_free_list to restore after test
	for (int i = 0; i <= MAX_POWER; i++) {
		free_list_backup[i] = GET_FREE_LIST_PTR(i);
	}

	size_t prev_size = 0;
	size_t curr_size = 4;
	size_t next_size = 16;
	size_t total_needed_size = prev_size + curr_size + next_size + HDR_FTR_SIZE*3;
	size_t new_size;

	char **ptr = malloc(WORD_SIZE * total_needed_size);
	char **p_prev = ptr;
	char **p_curr = p_prev + HDR_FTR_SIZE + prev_size;
	char **p_next = p_curr + HDR_FTR_SIZE + curr_size;

	// check for case when both prev and next are taken
	PUT_WORD(p_prev, PACK(prev_size, TAKEN));
	PUT_WORD(FTRP(p_prev), PACK(prev_size, TAKEN));
	PUT_WORD(p_curr, PACK(curr_size, FREE));
	PUT_WORD(FTRP(p_curr), PACK(curr_size, FREE));
	PUT_WORD(p_next, PACK(next_size, TAKEN));
	PUT_WORD(FTRP(p_next), PACK(next_size, TAKEN));

	coalesce(p_curr);
	assert(GET_SIZE(p_curr) == curr_size);
	assert(GET_SIZE(FTRP(p_curr)) == curr_size);
	assert(GET_SIZE(p_prev) == prev_size);
	assert(GET_SIZE(FTRP(p_prev)) == prev_size);
	assert(GET_SIZE(p_next) == next_size);
	assert(GET_SIZE(FTRP(p_next)) == next_size);
	assert(GET_STATUS(p_prev) == TAKEN);
	assert(GET_STATUS(p_next) == TAKEN);

	// check for case when prev taken and next free
	PUT_WORD(p_prev, PACK(prev_size, TAKEN));
	PUT_WORD(FTRP(p_prev), PACK(prev_size, TAKEN));
	PUT_WORD(p_curr, PACK(curr_size, FREE));
	PUT_WORD(FTRP(p_curr), PACK(curr_size, FREE));
	PUT_WORD(p_next, PACK(next_size, FREE));
	PUT_WORD(FTRP(p_next), PACK(next_size, FREE));

	place_block_into_free_list(p_next);

	coalesce(p_curr);
	new_size = curr_size + next_size + HDR_FTR_SIZE;
	assert(GET_SIZE(p_curr) == new_size);
	assert(GET_SIZE(FTRP(p_curr)) == new_size);
	assert(GET_SIZE(p_prev) == prev_size);
	assert(GET_SIZE(FTRP(p_prev)) == prev_size);
	assert(GET_STATUS(p_prev) == TAKEN);
	// check also to see that it was removed from free list
	assert(find_free_block(next_size) == NULL);

	// check for case when prev free and next taken
	PUT_WORD(p_prev, PACK(prev_size, FREE));
	PUT_WORD(FTRP(p_prev), PACK(prev_size, FREE));
	PUT_WORD(p_curr, PACK(curr_size, FREE));
	PUT_WORD(FTRP(p_curr), PACK(curr_size, FREE));
	PUT_WORD(p_next, PACK(next_size, TAKEN));
	PUT_WORD(FTRP(p_next), PACK(next_size, TAKEN));

	place_block_into_free_list(p_prev);

	coalesce(p_curr);
	new_size = prev_size + curr_size + HDR_FTR_SIZE;
	assert(GET_SIZE(p_prev) == new_size);
	assert(GET_SIZE(FTRP(p_prev)) == new_size);
	assert(GET_STATUS(p_next) == TAKEN);
	assert(GET_SIZE(p_next) == next_size);
	assert(GET_SIZE(FTRP(p_next)) == next_size);

	// check for case when both prev and next are free
	PUT_WORD(p_prev, PACK(prev_size, FREE));
	PUT_WORD(FTRP(p_prev), PACK(prev_size, FREE));
	PUT_WORD(p_curr, PACK(curr_size, FREE));
	PUT_WORD(FTRP(p_curr), PACK(curr_size, FREE));
	PUT_WORD(p_next, PACK(next_size, FREE));
	PUT_WORD(FTRP(p_next), PACK(next_size, FREE));

	place_block_into_free_list(p_prev);
	place_block_into_free_list(p_next);

	coalesce(p_curr);
	new_size = prev_size + curr_size + next_size + 2*HDR_FTR_SIZE;
	assert(GET_SIZE(p_prev) == new_size);
	assert(GET_SIZE(FTRP(p_prev)) == new_size);
	// check also to see that it was removed from free list
	assert(find_free_block(next_size) == NULL);

	// Restoring the value in main_free_list so that test doesn't have harmful side effects
	for (int i = 0; i <= MAX_POWER; i++) {
		SET_FREE_LIST_PTR(i, free_list_backup[i]);
	}

	printf("Coalesce test passed.\n\n");
}

static void test_alloc_free_block() {
	printf("Testing alloc_free_block.\n");

	char *free_list_backup[MAX_POWER + 1]; // backing up main_free_list to restore after test
	for (int i = 0; i <= MAX_POWER; i++) {
		free_list_backup[i] = GET_FREE_LIST_PTR(i);
	}

	// coalesce check variables
	size_t prev_size;
	size_t next_size;
	size_t total_block_size;
	size_t new_size;
	size_t extra_space;

	char **p_prev;
	char **p_next;


	initialize_free_lists_for_test();

	size_t needed_size;
	size_t free_block_size;
	size_t tot_free_block_size;

	char **total_block;
	char **free_block;

	// Case 1: alloc free block that is exact size
	free_block_size = 50;
	needed_size = 50;
	tot_free_block_size = free_block_size + HDR_FTR_SIZE;
	free_block = malloc(WORD_SIZE*tot_free_block_size);
	PUT_WORD(free_block, PACK(free_block_size, FREE));
	PUT_WORD(FTRP(free_block), PACK(free_block_size, FREE));

	alloc_free_block(free_block, needed_size);
	assert(GET_SIZE(free_block) == needed_size);
	assert(GET_STATUS(free_block) == TAKEN);
	assert(GET_SIZE(FTRP(free_block)) == needed_size);
	assert(GET_STATUS(FTRP(free_block)) == TAKEN);

	free(free_block);

	// Case 2: alloc free block that is larger than needed space by 2 words
	free_block_size = 52;
	needed_size = 50;
	tot_free_block_size = free_block_size + HDR_FTR_SIZE;
	free_block = malloc(WORD_SIZE*tot_free_block_size);
	PUT_WORD(free_block, PACK(free_block_size, FREE));
	PUT_WORD(FTRP(free_block), PACK(free_block_size, FREE));

	alloc_free_block(free_block, needed_size);
	assert(GET_SIZE(free_block) == free_block_size);
	assert(GET_STATUS(free_block) == TAKEN);
	assert(GET_SIZE(FTRP(free_block)) == free_block_size);
	assert(GET_STATUS(FTRP(free_block)) == TAKEN);

	free(free_block);

	// Case 3: alloc free block that is larger than needed space
	free_block_size = 116;
	needed_size = 50;
	tot_free_block_size = free_block_size + HDR_FTR_SIZE;
	prev_size = 10;
	next_size = 10;
	total_block_size = prev_size + free_block_size + next_size + HDR_FTR_SIZE*3;
	total_block = malloc(WORD_SIZE*total_block_size);
	free_block = total_block + prev_size + HDR_FTR_SIZE;
	PUT_WORD(free_block, PACK(free_block_size, FREE));
	PUT_WORD(FTRP(free_block), PACK(free_block_size, FREE));

	// setup for coalesce
	p_prev = total_block;
	p_next = free_block + tot_free_block_size;
	PUT_WORD(p_prev, PACK(prev_size, TAKEN));
	PUT_WORD(FTRP(p_prev), PACK(prev_size, TAKEN));
	PUT_WORD(p_next, PACK(next_size, TAKEN));
	PUT_WORD(FTRP(p_next), PACK(next_size, TAKEN));

	alloc_free_block(free_block, needed_size);
	assert(GET_SIZE(free_block) == needed_size);
	assert(GET_STATUS(free_block) == TAKEN);
	assert(GET_SIZE(FTRP(free_block)) == needed_size);
	assert(GET_STATUS(FTRP(free_block)) == TAKEN);
	assert(GET_SIZE(p_prev) == prev_size);
	assert(GET_STATUS(p_prev) == TAKEN);
	assert(GET_SIZE(FTRP(p_prev)) == prev_size);
	assert(GET_STATUS(FTRP(p_prev)) == TAKEN);
	assert(GET_SIZE(p_next) == next_size);
	assert(GET_STATUS(p_next) == TAKEN);
	assert(GET_SIZE(FTRP(p_next)) == next_size);
	assert(GET_STATUS(FTRP(p_next)) == TAKEN);
	extra_space = free_block_size - needed_size - HDR_FTR_SIZE;
	assert(GET_SIZE(find_free_block(extra_space)) == extra_space);

	free(total_block);

	// Case 4: alloc free block that is larger and can coalesce
	free_block_size = 116;
	needed_size = 50;
	tot_free_block_size = free_block_size + HDR_FTR_SIZE;
	prev_size = 10;
	next_size = 10;
	total_block_size = prev_size + free_block_size + next_size + HDR_FTR_SIZE*3;
	total_block = malloc(WORD_SIZE*total_block_size);
	free_block = total_block + prev_size + HDR_FTR_SIZE;
	PUT_WORD(free_block, PACK(free_block_size, FREE));
	PUT_WORD(FTRP(free_block), PACK(free_block_size, FREE));

	// setup for coalesce
	p_prev = total_block;
	p_next = free_block + tot_free_block_size;
	PUT_WORD(p_prev, PACK(prev_size, TAKEN));
	PUT_WORD(FTRP(p_prev), PACK(prev_size, TAKEN));
	PUT_WORD(p_next, PACK(next_size, FREE));
	PUT_WORD(FTRP(p_next), PACK(next_size, FREE));
	place_block_into_free_list(p_next);

	alloc_free_block(free_block, needed_size);
	assert(GET_SIZE(free_block) == needed_size);
	assert(GET_STATUS(free_block) == TAKEN);
	assert(GET_SIZE(FTRP(free_block)) == needed_size);
	assert(GET_STATUS(FTRP(free_block)) == TAKEN);
	assert(GET_SIZE(p_prev) == prev_size);
	assert(GET_STATUS(p_prev) == TAKEN);
	assert(GET_SIZE(FTRP(p_prev)) == prev_size);
	assert(GET_STATUS(FTRP(p_prev)) == TAKEN);
	extra_space = free_block_size + HDR_FTR_SIZE + next_size - needed_size - HDR_FTR_SIZE;
	assert(GET_SIZE(find_free_block(extra_space)) == extra_space);

	free(total_block);

	// Restoring the value in main_free_list so that test doesn't have harmful side effects
	for (int i = 0; i <= MAX_POWER; i++) {
		SET_FREE_LIST_PTR(i, free_list_backup[i]);
	}

	printf("alloc_free_block test passed.\n\n");
}

int mm_check()
{
    test_find_free_list_index();
    test_ALIGN();
    test_EVENIZE();
    test_GET_SIZE();
    test_GET_STATUS();
    test_PACK();
    test_PUT_WORD();
    test_FTRP();
    test_MAIN_FREE_LIST_INIT();
    test_SET_PTR();
    test_GET_PTR_PRED_FIELD();
    test_GET_PTR_SUCC_FIELD();
    test_GET_SUCC();
	test_GET_PRED();
    test_place_block_into_free_list();
    test_remove_block_from_free_list();
	test_PREV_IN_HEAP();
	test_NEXT_IN_HEAP();
    test_find_free_block();
		test_PREV_IN_HEAP();
		test_NEXT_IN_HEAP();
		test_coalesce();
		test_alloc_free_block();
    // test this last
	test_extend_heap();
}
