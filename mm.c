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

static char *main_free_list[MAX_POWER + 1];

static char *heap_ptr;

// Function Declarations
static size_t find_free_list_index(size_t words);

static void *extend_heap(size_t words);
static void test_extend_heap();

static void *coalesce(void *bp);
static void *find_free_block(size_t words);
static void alloc_free_block(void *bp, size_t words);
static void place_block_into_free_list(void *bp);
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
	Returns the pointer to the block.
	Returns NULL if block large enough is not found.
*/
static void *find_free_block(size_t words) {

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
static void place_block_into_free_list(void *bp) {

}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
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
}
