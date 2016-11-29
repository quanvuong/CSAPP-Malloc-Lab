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
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE     4          // word and header/footer size (bytes)
#define DSIZE     8          // double word size (bytes)
#define INITCHUNKSIZE (1<<6)
#define CHUNKSIZE (1<<12)

#define SEG_LIST_NUM     20      
#define REALLOC_BUFFER  (1<<7)    

#define MAX(x, y) ((x) > (y) ? (x) : (y)) 
#define MIN(x, y) ((x) < (y) ? (x) : (y)) 

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

static void *coalesce(void *bp);
static void * extend_heap(size_t bytes);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
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
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
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

    return bp;
}

static void * extend_heap(size_t bytes)
{
    void * ptr;
    size_t size = ALIGN(bytes);

    if ((ptr = mem_sbrk(size)) == (void *) -1)
        return NULL;
    
    // Initialize the new block header, footer, and new epilog
    PUT(HDRP(ptr), PACK(size, 0)); // header
    PUT(FTRP(ptr), PACK(size, 0)); // footer
    PUT(HDRP(NEXT_BLKP(ptr)), PACK(0, 1)); // epilog, size 1, allocated

    add_free_block(ptr);

    return coalesce(ptr);
}












