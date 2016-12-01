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
    "SuperAllocator",
    /* First member's full name */
    "Quan Vuong",
    /* First member's email address */
    "qhv200@nyu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
<<<<<<< HEAD
<<<<<<< HEAD
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)
=======
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

=======
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

>>>>>>> parent of e473500... working version from the book
#define WSIZE     4          // word and header/footer size (bytes)
#define DSIZE     8          // double word size (bytes)
#define INITCHUNKSIZE (1<<6)
#define CHUNKSIZE (1<<12)
>>>>>>> parent of e473500... working version from the book

#define SEG_LIST_NUM     20      
#define REALLOC_BUFFER  (1<<7)    

#define MAX(x, y) ((x) > (y) ? (x) : (y)) 
#define MIN(x, y) ((x) < (y) ? (x) : (y)) 
<<<<<<< HEAD

// Pack a size and allocated bit into a word
#define PACK(size, alloc) ((size) | (alloc))

// Read and write a word at address p 
#define GET(p)      (*(unsigned int *)(p))
#define PUT(p, val)     (*(unsigned int *)(p) = (val))

<<<<<<< HEAD
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

// Address of free block's predecessor and successor entries
#define PRED_PTR(ptr) ((char *)(ptr))
#define SUCC_PTR(ptr) ((char *)(ptr) + WSIZE)

// Address of free block's predecessor and successor on the segregated list
#define PRED(ptr) (*(char **)(ptr))
#define SUCC(ptr) (*(char **)(SUCC_PTR(ptr)))

#define BP_SMALLER(bp, size) (GET_SIZE(HDRP(bp)) < size)
#define BP_LARGER_EQUAL(bp, size) (GET_SIZE(HDRP(bp)) >= size)

// Store predecessor or successor pointer for free blocks
#define SET_PTR(p, ptr) (*(char **)(p) = (char *)(ptr))

#define NUM_FREE_LIST 20

// Global variables and funcion declarations
static char *heap_listp;
char *free_lists[NUM_FREE_LIST];
static void add_free_block(char *bp);
static void *coalesce(void *bp);
static void *find_fit(size_t size);
static void *extend_heap(size_t words);
static int free_list_index(size);
static void remove_free_block(void *bp);
static void place(void *bp, size_t asize);

static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc)
    {
	return bp;
    }

	remove_free_block(bp);

    if (prev_alloc && !next_alloc)
    {
	remove_free_block(NEXT_BLKP(bp));
	size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc)
    {
	remove_free_block(PREV_BLKP(bp));
	size += GET_SIZE(HDRP(PREV_BLKP(bp)));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
    }
    else
    {
	remove_free_block(NEXT_BLKP(bp));
	remove_free_block(PREV_BLKP(bp));
	size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
    }

    add_free_block(bp);

    return bp;
}

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
    {
	return NULL;
    }

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}

static void *find_fit(size_t size)
{
    int list = free_list_index(size);

    void *free_block, *succ;

    for (; list < NUM_FREE_LIST; list++)
    {
	free_block = free_lists[list];

	if (free_block == NULL)
	{
	    continue;
	} // No free block in this list

	// int flp_size = SIZE(free_block);

	if (BP_SMALLER(free_block, size))
	{ // No free block that fits, free list is descend-sorted
	    continue;
	}

	while ((succ = SUCC(free_block)) != NULL)
	{ // Traverse the free list
	    if (BP_SMALLER(succ, size))
	    { // Found best fit
		return free_block;
	    }

	    free_block = succ;
	}

	// Reach end of list, just take the smallest free block
	if (BP_LARGER_EQUAL(free_block, size))
	    return free_block;
    }

    return NULL;
}

static int free_list_index(size)
{
    int list = 0;

    while ((list <= NUM_FREE_LIST - 1) && (size > 1))
    {
	size >>= 1;
	list++;
    }

    return list;
}

static void add_free_block(char *bp)
{
    unsigned int bp_size = GET_SIZE(HDRP(bp));
    int list = free_list_index(bp_size);

    void *front_ptr = free_lists[list];
    void *back_ptr = NULL;

    // If the free list is empty
    if (front_ptr == NULL)
    {
	SET_PTR(PRED_PTR(bp), NULL);
	SET_PTR(SUCC_PTR(bp), NULL);
	free_lists[list] = bp;
	return;
    }

    // If the new block is the biggest in the respective free list
    int front_ptr_size = GET_SIZE(HDRP(front_ptr));

    if (BP_LARGER_EQUAL(bp, GET_SIZE(HDRP(front_ptr))))
    {
	free_lists[list] = bp;
	SET_PTR(PRED_PTR(bp), NULL);
	SET_PTR(SUCC_PTR(bp), front_ptr);
	SET_PTR(PRED_PTR(front_ptr), bp);
	return;
    }

    // Keep each free list sorted in descending order of size
    // int fp_size = SIZE(front_ptr);
    // int first = front_ptr != NULL ? 1 : 0;
    // int second = BP_LARGER_EQUAL(front_ptr, bp_size) ? 1 : 0;

    while (front_ptr != NULL && BP_LARGER_EQUAL(front_ptr, bp_size))
    {
	back_ptr = front_ptr;
	front_ptr = SUCC(front_ptr);
    }

    // Reached the end of the free list
    if (front_ptr == NULL)
    {
	SET_PTR(SUCC_PTR(bp), NULL);
	SET_PTR(PRED_PTR(bp), back_ptr);
	SET_PTR(SUCC_PTR(back_ptr), bp);
    }
    else
    { // Haven't reached the end of the free list
	SET_PTR(SUCC_PTR(bp), front_ptr);
	SET_PTR(PRED_PTR(bp), back_ptr);
	SET_PTR(PRED_PTR(front_ptr), bp);
	SET_PTR(SUCC_PTR(back_ptr), bp);
    }
}

static void remove_free_block(void *bp)
{
    int size = GET_SIZE(HDRP(bp));
    int list = free_list_index(size);

    void *pred = PRED(bp);
    void *succ = SUCC(bp);

    if (pred == NULL)
    {
		if (succ == NULL) 
		{ // Both succ and pred null
			free_lists[list] = NULL;
		}
		else
		{ // succ not null, pred null
			free_lists[list] = succ;
			SET_PTR(SUCC_PTR(bp), NULL);
			SET_PTR(PRED_PTR(succ), NULL);
		}
    }
    else
    {
		if (succ != NULL)
		{ // both succ and pred not null
			SET_PTR(PRED_PTR(succ), pred);
			SET_PTR(SUCC_PTR(pred), succ);
			SET_PTR(PRED_PTR(bp), NULL);
			SET_PTR(SUCC_PTR(bp), NULL);
		}
		else
		{ // pred not null, succ null
			SET_PTR(SUCC_PTR(pred), NULL);
			SET_PTR(PRED_PTR(bp), NULL);
		}
    }
}

static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    remove_free_block(bp);

    if ((csize - asize) >= (2 * DSIZE))
    {
	PUT(HDRP(bp), PACK(asize, 1));
	PUT(FTRP(bp), PACK(asize, 1));
	bp = NEXT_BLKP(bp);
	PUT(HDRP(bp), PACK(csize - asize, 0));
	PUT(FTRP(bp), PACK(csize - asize, 0));
    }
    else
    {
	PUT(HDRP(bp), PACK(csize, 1));
	PUT(FTRP(bp), PACK(csize, 1));
    }
}
=======
// Store predecessor or successor pointer for free blocks 
#define SET_PTR(p, ptr) (*(unsigned int *)(p) = (unsigned int)(ptr))

// Read the size and allocation bit from address p 
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_TAG(p)   (GET(p) & 0x2)

// Address of block's header and footer 
#define HDRP(ptr) ((char *)(ptr) - WSIZE)
#define FTRP(ptr) ((char *)(ptr) + GET_SIZE(HDRP(ptr)) - DSIZE)

// Address of (physically) next and previous blocks 
#define NEXT_BLKP(ptr) ((char *)(ptr) + GET_SIZE((char *)(ptr) - WSIZE))
#define PREV_BLKP(ptr) ((char *)(ptr) - GET_SIZE((char *)(ptr) - DSIZE))

// Address of free block's predecessor and successor entries 
#define PRED_PTR(ptr) ((char *)(ptr))
#define SUCC_PTR(ptr) ((char *)(ptr) + WSIZE)

// Address of free block's predecessor and successor on the segregated list 
#define PRED(ptr) (*(char **)(ptr))
#define SUCC(ptr) (*(char **)(SUCC_PTR(ptr)))

#define BP_SMALLER(bp, size) (GET_SIZE(HDRP(bp)) < size)
#define BP_LARGER_EQUAL(bp, size) (GET_SIZE(HDRP(bp)) >= size)

// Global variables and function declarations
void * prolog_p; // point to the start of the ftr of prolog

static void * extend_heap(size_t bytes);
static void add_free_block(void *bp);
// static int which_free_list(size_t s);
static void remove_free_block(void *bp);
static void place(void *bp, size_t requested_size);
static void *find_fit(size_t size);
static void *coalesce(void *ptr);
>>>>>>> parent of e473500... working version from the book

=======

// Pack a size and allocated bit into a word
#define PACK(size, alloc) ((size) | (alloc))

// Read and write a word at address p 
#define GET(p)      (*(unsigned int *)(p))
#define PUT(p, val)     (*(unsigned int *)(p) = (val))

// Store predecessor or successor pointer for free blocks 
#define SET_PTR(p, ptr) (*(unsigned int *)(p) = (unsigned int)(ptr))

// Read the size and allocation bit from address p 
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_TAG(p)   (GET(p) & 0x2)

// Address of block's header and footer 
#define HDRP(ptr) ((char *)(ptr) - WSIZE)
#define FTRP(ptr) ((char *)(ptr) + GET_SIZE(HDRP(ptr)) - DSIZE)

// Address of (physically) next and previous blocks 
#define NEXT_BLKP(ptr) ((char *)(ptr) + GET_SIZE((char *)(ptr) - WSIZE))
#define PREV_BLKP(ptr) ((char *)(ptr) - GET_SIZE((char *)(ptr) - DSIZE))

// Address of free block's predecessor and successor entries 
#define PRED_PTR(ptr) ((char *)(ptr))
#define SUCC_PTR(ptr) ((char *)(ptr) + WSIZE)

// Address of free block's predecessor and successor on the segregated list 
#define PRED(ptr) (*(char **)(ptr))
#define SUCC(ptr) (*(char **)(SUCC_PTR(ptr)))

#define BP_SMALLER(bp, size) (GET_SIZE(HDRP(bp)) < size)
#define BP_LARGER_EQUAL(bp, size) (GET_SIZE(HDRP(bp)) >= size)

// Global variables and function declarations
void * prolog_p; // point to the start of the ftr of prolog

static void * extend_heap(size_t bytes);
static void add_free_block(void *bp);
// static int which_free_list(size_t s);
static void remove_free_block(void *bp);
static void place(void *bp, size_t requested_size);
static void *find_fit(size_t size);
static void *coalesce(void *ptr);

>>>>>>> parent of e473500... working version from the book
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
<<<<<<< HEAD
<<<<<<< HEAD
    for (int i = 0; i < NUM_FREE_LIST; i++)
    {
	free_lists[i] = NULL;
    }

    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
    {
	return -1;
    }
    PUT(heap_listp, 0);
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));
    heap_listp += (2 * WSIZE);

    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
    {
	return -1;
    }
=======
=======
>>>>>>> parent of e473500... working version from the book
    /* Create heap basic prologue, epilogue */
    if ((prolog_p = mem_sbrk(4*WSIZE)) == (void *) -1) return -1;

    PUT(prolog_p, 0); /* Empty padding block */
    PUT(prolog_p + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(prolog_p + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(prolog_p + (3*WSIZE), PACK(0, 1)); /* Epilogue header */
    prolog_p += (2*WSIZE); /* Point prolog_p to Prologue */

    /* Allocate additional space to the heap */
    if (extend_heap(CHUNKSIZE) == NULL) 
        return -1;
<<<<<<< HEAD
>>>>>>> parent of e473500... working version from the book
=======
>>>>>>> parent of e473500... working version from the book

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
<<<<<<< HEAD
<<<<<<< HEAD
    size_t asize;
    size_t extendsize;
    char *bp;

    if (size == 0)
    {
	return NULL;
    }

    if (size <= DSIZE)
    {
	asize = 2 * DSIZE;
    }
    else
    {
	asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }

    if ((bp = find_fit(asize)) != NULL)
    {
	place(bp, asize);
	return bp;
    }

    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
    {
	return NULL;
    }
    place(bp, asize);
=======
=======
>>>>>>> parent of e473500... working version from the book
    if (size == 0) return NULL;

    size_t extend_size;
    size_t asize;
    void * bp;

    // Adjust block size to include overhead and alignment reqs 
    if (size <= DSIZE) asize = DSIZE * 2;
    else {
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }

    /* Search free lists for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. We get more memory and place the block */
    extend_size = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extend_size)) == NULL) 
        return NULL;
    place(bp, asize);

<<<<<<< HEAD
>>>>>>> parent of e473500... working version from the book
=======
>>>>>>> parent of e473500... working version from the book
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
<<<<<<< HEAD
<<<<<<< HEAD
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

    // add_free_block(ptr);

    coalesce(ptr);

    return;
=======
    unsigned int size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0)); 
	PUT(FTRP(bp), PACK(size, 0));
>>>>>>> parent of e473500... working version from the book
=======
    unsigned int size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0)); 
	PUT(FTRP(bp), PACK(size, 0));
>>>>>>> parent of e473500... working version from the book
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
<<<<<<< HEAD
<<<<<<< HEAD
	return NULL;
    copySize = *(size_t *)((char *)oldptr - WSIZE);
=======
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
>>>>>>> parent of e473500... working version from the book
=======
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
>>>>>>> parent of e473500... working version from the book
    if (size < copySize)
	copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

static void *coalesce(void *bp)
{
    int size_prev_block = GET_SIZE((char *)(bp) - DSIZE);
    void * prev_block = (void *) ((char *)(bp) - size_prev_block);

    // void * prev_block = PREV_BLKP(bp);
    size_t prev_alloc = GET_ALLOC(FTRP(prev_block));
    void * next_block = NEXT_BLKP(bp);
    size_t next_alloc = GET_ALLOC(HDRP(next_block));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc)
    { /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc)
    { /* Case 2 */
        remove_free_block(bp);
        remove_free_block(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if (!prev_alloc && next_alloc)
    { /* Case 3 */
        remove_free_block(bp);
        remove_free_block(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    else
    { /* Case 4 */
        remove_free_block(bp);
        remove_free_block(NEXT_BLKP(bp));
        remove_free_block(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    add_free_block(bp);

    return bp;
}

static void * extend_heap(size_t bytes)
{
    void * bp;
    size_t size = ALIGN(bytes);

    if ((bp = mem_sbrk(size)) == (void *) -1)
        return NULL;
    
    // Initialize the new block header, footer, and new epilog
    PUT(HDRP(bp), PACK(size, 0)); // header
    PUT(FTRP(bp), PACK(size, 0)); // footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // epilog, size 1, allocated

    add_free_block(bp);

    return coalesce(bp);
}

static void *find_fit(size_t size)
{
    void * bp = NEXT_BLKP(prolog_p);

    while (GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) < size && GET_SIZE(HDRP(bp)) > 0) 
        bp = NEXT_BLKP(bp);

    if (GET_SIZE(HDRP(bp))) return NULL;

    return bp;
}

static void add_free_block(void *bp)
<<<<<<< HEAD
{
<<<<<<< HEAD
=======
    unsigned int size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0)); 
	PUT(FTRP(bp), PACK(size, 0));
>>>>>>> parent of e473500... working version from the book
}

static void remove_free_block(void *bp)
{
    unsigned int size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 1)); 
	PUT(FTRP(bp), PACK(size, 1));
}

=======
{
    unsigned int size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0)); 
	PUT(FTRP(bp), PACK(size, 0));
}

static void remove_free_block(void *bp)
{
    unsigned int size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 1)); 
	PUT(FTRP(bp), PACK(size, 1));
}

>>>>>>> parent of e473500... working version from the book
static void place(void *bp, size_t requested_size)
{
    remove_free_block(bp);

    PUT(HDRP(bp), PACK(requested_size, 1));
    PUT(FTRP(bp), PACK(requested_size, 1));
}


// static void *coalesce(void *ptr)
// {
//   size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(ptr)));
//   size_t next_alloc = GET_ALLOC(HDRP(PREV_BLKP(ptr)));
//   size_t size = GET_SIZE(HDRP(ptr));
  
//   /* Return if previous and next blocks are allocated */
//   if (prev_alloc && next_alloc) {
//     return ptr;
//   }
  
//   /* Do not coalesce with previous block if it is tagged */
//   if (GET_TAG(HDRP(PREV_BLKP(ptr))))
//     prev_alloc = 1;
  
//   /* Remove old block from list */
//   remove_free_block(ptr);
  
//   /* Detect free blocks and merge, if possible */
//   if (prev_alloc && !next_alloc) {
//     remove_free_block(PREV_BLKP(ptr));
//     size += GET_SIZE(HDRP(PREV_BLKP(ptr)));
//     PUT(HDRP(ptr), PACK(size, 0));
//     PUT(FTRP(ptr), PACK(size, 0));
//   } else if (!prev_alloc && next_alloc) {
//     remove_free_block(PREV_BLKP(ptr));
//     size += GET_SIZE(HDRP(PREV_BLKP(ptr)));
//     PUT(FTRP(ptr), PACK(size, 0));
//     PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
//     ptr = PREV_BLKP(ptr);
//   } else {
//     remove_free_block(PREV_BLKP(ptr));
//     remove_free_block(PREV_BLKP(ptr));
//     size += GET_SIZE(HDRP(PREV_BLKP(ptr))) + GET_SIZE(HDRP(PREV_BLKP(ptr)));
//     PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
//     PUT(FTRP(PREV_BLKP(ptr)), PACK(size, 0));
//     ptr = PREV_BLKP(ptr);
//   }
  
//   /* Adjust segregated linked lists */
//   add_free_block(ptr);
  
//   return ptr;
// }

