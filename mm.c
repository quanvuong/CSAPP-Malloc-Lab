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
#define CHUNK (1<<12) /* extend heap by this amount (bytes) */
#define STATUS_BIT_SIZE 3 // bits
#define HDR_FTR_SIZE 2 // in words
#define HDR_SIZE 1 // in words
#define FTR_SIZE 1 // in words
#define PRED_FIELD_SIZE 1 // in words

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

// Pack a size and allocated bit into a wBIT_ord
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
#define GET_SUCC(bp) (*(GET_PTR_SUCC_FIELD(bp)))

// Global variables
static char *main_free_list[MAX_POWER + 1];
static char *heap_ptr;

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
*/
static void *coalesce(void *bp) {

}

/*
	Relies on mem_sbrk to create a new free block.
	Does not coalesce.
	Returns pointer to the new block of memory with
	header and footer already defined.
	Returns NULL if we ran out of physical memory.
*/
static void *extend_heap(size_t words) {

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
	while (GET_FREE_LIST_PTR(index) == NULL && index <= MAX_POWER) {
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
	Assume that size in words given is <= the size of the block at input.
	bp input is the block that you found already that is large enough.
	The function reduces the size of the block if it is too large.
	The remaining size is either placed in free_list or left hanging if it is 0.
*/
static void alloc_free_block(void *bp, size_t words) {

}

/*
	Places the block into the free list based on block size.
*/
static void place_block_into_free_list(char **bp) {
    size_t size = GET_SIZE(bp);
    int index = find_free_list_index(size);

    char **front_ptr = main_free_list[index];
    char **back_ptr = NULL;

    // If the free list is empty
    if (front_ptr == NULL)
    {
        SET_PTR(GET_PTR_SUCC_FIELD(bp), NULL);
        main_free_list[index] = bp;
        return;
    }

    // If the new block is the biggest in the respective free list
    if (size >= GET_SIZE(front_ptr))
    {
        main_free_list[index] = bp;
        SET_PTR(GET_PTR_SUCC_FIELD(bp), front_ptr);
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
        SET_PTR(GET_PTR_SUCC_FIELD(bp), NULL);
        return;
    }
    else
    { // Haven't reached the end of the free list
        SET_PTR(GET_PTR_SUCC_FIELD(back_ptr), bp);
        SET_PTR(GET_PTR_SUCC_FIELD(bp), front_ptr);
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
        main_free_list[i] = NULL;
    }

    mm_check();
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{

}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{

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
    char **ptr = malloc(WORD_SIZE*test_block_size);
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
    char **bp = malloc(2); // enough for hdr + pred field
    char **ptr_to_pred_field = bp + HDR_SIZE; // bp should point to the PRED field after this line

    assert(GET_PTR_PRED_FIELD(bp) == ptr_to_pred_field);
    printf("Test passed.\n\n");
    free(bp);
}

static void test_GET_PTR_SUCC_FIELD()
{
    printf("Test GET_PTR_SUCC_FIELD.\n");
    char **bp = malloc(3); // enough for hdr + pred field + succ field
    char **ptr_to_succ_field = bp + HDR_SIZE + PRED_FIELD_SIZE; // bp should point to the succ field after this line

    assert(GET_PTR_SUCC_FIELD(bp) == ptr_to_succ_field);
    printf("Test passed.\n\n");
    free(bp);
}


static void test_GET_SUCC()
{
    printf("Test test_GET_SUCC.\n");
    char **bp = malloc(3); // enough for hdr + pred field + succ field
    char **ptr_to_succ_field = bp + HDR_SIZE + PRED_FIELD_SIZE; // bp should point to the succ field after this line

    assert(GET_SUCC(bp) == (*ptr_to_succ_field));
    printf("Test passed.\n\n");
    free(bp);
}

// Initialize main_free_list to all NULL
// Insert 3 block of size 60, 52, 40 into the free list
static void test_place_block_into_free_list_helper()
{
    for (int i = 0; i <= MAX_POWER; i++) {
        main_free_list[i] = NULL;
    }

    int block_1_size = 60;
    int block_2_size = 53;
    int block_3_size = 40;
    int index = find_free_list_index(block_1_size);

    // Building the list
    char **block_1 = malloc((block_1_size+HDR_FTR_SIZE)*WORD_SIZE);
    PUT_WORD(block_1, PACK(block_1_size, FREE));
    PUT_WORD(FTRP(block_1), PACK(block_1_size, FREE));
    main_free_list[index] = block_1;

    char **block_2 = malloc((block_2_size+HDR_FTR_SIZE)*WORD_SIZE);
    PUT_WORD(block_2, PACK(block_2_size, FREE));
    PUT_WORD(FTRP(block_2), PACK(block_2_size, FREE));
    SET_PTR(GET_PTR_SUCC_FIELD(block_1), block_2);

    char **block_3 = malloc((block_3_size+HDR_FTR_SIZE)*WORD_SIZE);
    PUT_WORD(block_3, PACK(block_3_size, FREE));
    PUT_WORD(FTRP(block_3), PACK(block_3_size, FREE));
    SET_PTR(GET_PTR_SUCC_FIELD(block_2), block_3);
    SET_PTR(GET_PTR_SUCC_FIELD(block_3), NULL);
}

// Case 2: Free list not empty. New block is the smallest block in list.
static void test_place_block_into_free_list_case_2()
{
    test_place_block_into_free_list_helper();

    // New block
    int new_b_size = 35;
    char **new_block = malloc((new_b_size+HDR_FTR_SIZE)*WORD_SIZE);
    PUT_WORD(new_block, PACK(new_b_size, FREE));
    PUT_WORD(FTRP(new_block), PACK(new_b_size, FREE));

    int index = find_free_list_index(new_b_size);

    char **end_of_list = main_free_list[index];

    while (GET_SUCC(end_of_list) != NULL) {
        end_of_list = GET_SUCC(end_of_list);
    }

    place_block_into_free_list(new_block);

    assert(GET_SUCC(end_of_list) == new_block);
    assert(GET_SUCC(new_block) == NULL);
    free(new_block);
}

// Case 3: free list is not empty. New block size is neither the largest or smallest in list.
static void test_place_block_into_free_list_case_3()
{
    test_place_block_into_free_list_helper();

    // New block
    int new_b_size = 50;
    char **new_block = malloc((new_b_size+HDR_FTR_SIZE)*WORD_SIZE);
    PUT_WORD(new_block, PACK(new_b_size, FREE));
    PUT_WORD(FTRP(new_block), PACK(new_b_size, FREE));

    int index = find_free_list_index(new_b_size);

    char **front_ptr = main_free_list[index];
    char **back_ptr = NULL;

    while (GET_SIZE(front_ptr) > new_b_size) {
        back_ptr = front_ptr;
        front_ptr = GET_SUCC(front_ptr);
    }

    place_block_into_free_list(new_block);

    assert(GET_SUCC(back_ptr) == new_block);
    assert(GET_SUCC(new_block) == front_ptr);
    free(new_block);
}


// Case 4: Free list not empty. New block is the biggest block in list
static void test_place_block_into_free_list_case_4()
{
    test_place_block_into_free_list_helper();

    // New Block
    int new_b_size = 60;
    char **new_block = malloc((new_b_size+HDR_FTR_SIZE)*WORD_SIZE);
    PUT_WORD(new_block, PACK(new_b_size, FREE));
    PUT_WORD(FTRP(new_block), PACK(new_b_size, FREE));

    int index = find_free_list_index(new_b_size);
    char **previous_biggest = main_free_list[index];

    place_block_into_free_list(new_block);

    assert(main_free_list[index] == new_block);
    assert(GET_SUCC(new_block) == previous_biggest);
    free(new_block);
}


static void test_place_block_into_free_list()
{
    printf("Test place_block_into_free_list.\n");

    char *free_list_backup[MAX_POWER + 1]; // backing up main_free_list to restore after test
    for (int i = 0; i <= MAX_POWER; i++) {
        free_list_backup[i] = main_free_list[i];
    }

    // Case 1: Free list currently empty
    for (int i = 0; i <= MAX_POWER; i++) {
        main_free_list[i] = NULL;
    }

    int new_block_size = 20; // words
    char **bp = malloc((new_block_size+HDR_FTR_SIZE)*WORD_SIZE);
    PUT_WORD(bp, PACK(new_block_size, FREE));
    PUT_WORD(FTRP(bp), PACK(new_block_size, FREE));

    place_block_into_free_list(bp);

    int list = find_free_list_index(GET_SIZE(bp));
    assert(main_free_list[list] == bp);
    assert(GET_SUCC(bp) == NULL);
    free(bp);

    // Case 2: Free list not empty. New block is the smallest block in list.
    test_place_block_into_free_list_case_2();

    // Case 3: free list is not empty. New block size is neither the largest or smallest in list.
    test_place_block_into_free_list_case_3();

    // Case 4: Free list not empty. New block is the biggest block in list
    test_place_block_into_free_list_case_4();

    // Restoring the value in main_free_list so that test doesn't have harmful side effects
    for (int i = 0; i <= MAX_POWER; i++) {
        main_free_list[i] = free_list_backup[i];
    }

    printf("Test passed.\n\n");
}

static void test_find_free_block() {
	printf("Test find_free_block.\n");

	char *free_list_backup[MAX_POWER + 1]; // backing up main_free_list to restore after test
	for (int i = 0; i <= MAX_POWER; i++) {
		free_list_backup[i] = GET_FREE_LIST_PTR(i);
	}

	char **bp;

	test_place_block_into_free_list_helper();

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

	// Restoring the value in main_free_list so that test doesn't have harmful side effects
	for (int i = 0; i <= MAX_POWER; i++) {
		SET_FREE_LIST_PTR(i, free_list_backup[i]);
	}

	printf("Test passed.\n\n");
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
    test_place_block_into_free_list();
		test_find_free_block();
}
