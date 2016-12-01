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
#define TAKEN 0
#define FREE 1

#define WORD_SIZE 4 /* bytes */
#define CHUNK (1<<12) /* extend heap by this amount (bytes) */

#define GET_BYTE(p) (*(char *)(p))
#define GET_WORD(p) (*(unsigned int *)(p))

// TODO: Add macros to be defined and tested


static char *main_free_list[MAX_POWER + 1];

static char *heap_ptr;

// Function Declarations
static size_t find_free_list_index(size_t words);
static void test_find_free_list_index();

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
    test_find_free_list_index();
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
    int index_2 = find_free_list_index(5);
    int index_3 = find_free_list_index(15);
    int index_10 = find_free_list_index(1024);
    int index_11 = find_free_list_index(2048);

    assert(index_2 == 2);
    assert(index_3 == 3);
    assert(index_10 == 10);
    assert(index_11 == 11);
}

int mm_check()
{

}
