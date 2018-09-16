#include <stdio.h>  // needed for size_t
#include <unistd.h> // needed for sbrk
#include <assert.h> // needed for asserts
#include "dmm.h"

/* You can improve the below metadata structure using the concepts from Bryant
 * and OHallaron book (chapter 9).
 */

typedef struct metadata {
  /* size_t is the return type of the sizeof operator. Since the size of an
   * object depends on the architecture and its implementation, size_t is used
   * to represent the maximum size of any object in the particular
   * implementation. size contains the size of the data object or the number of
   * free bytes
   */
  size_t size;
  struct metadata* next;
  struct metadata* prev; 
} metadata_t;

/* freelist maintains all the blocks which are not in use; freelist is kept
 * sorted to improve coalescing efficiency 
 */

static metadata_t* freelist = NULL;
bool heapFull = false;

void* dmalloc(size_t numbytes) {
  // printf("%s\n", "Start dmalloc");
  
  /* Check to make sure heap isn't full */
  if (heapFull) {
    printf("%s\n", "Heap is full");  
    return NULL;
  }

  /* initialize through sbrk call first time */
  if(freelist == NULL) { 			
    if(!dmalloc_init()) {
      return NULL;
    }
  }

  assert(numbytes > 0);

  /* your code here */
  size_t total_Block_Size = METADATA_T_ALIGNED + ALIGN(numbytes);
  // size_t total_Block_Size2 = total_Block_Size;

  metadata_t* fL_ptr = freelist;
  metadata_t* fL_ptr_found;

  /* Find the block to allocate memory into */
  bool finished = false;
  while (!finished) {
    // printf("%s\n", "while loop 3");
    /* freelist is empty */
    if (freelist == NULL) {
      printf("%s\n", "freelist is NULL"); 
      finished = true;
      heapFull = true;
      
    /* This block is too small for the malloced size */
    } else if (fL_ptr->size < total_Block_Size) {
      /* This is the last block, no block is big enough */
      if (fL_ptr->next == NULL) {
        printf("%s\n", "no block is big enough"); 
        finished = true;
        
      /* Check next block */
      } else {
        fL_ptr = fL_ptr->next;
      }
      /* Found block */
    } else {
      printf("%s\n", "found block");  
      fL_ptr_found = fL_ptr;
      finished = true;
      
    }
  }
  size_t print_this = fL_ptr_found->size;
  printf("%s%zu\n", "found block size: ", print_this);
  
  /* The malloced size exactly fits the found block */
  if (fL_ptr_found->size == total_Block_Size) {
    printf("%s\n", "malloc fits exactly"); 

    /* First free block is being filled */
    if (fL_ptr_found == freelist) {
      /* This is the last block, set fL to NULL, heap is full! */
      if (fL_ptr_found->next == NULL) {
        printf("%s\n", "1st block & last block"); 
        freelist = NULL;
        freelist->next = NULL;
        freelist->prev = NULL;
        printf("%s\n", "heapFull from 1st block & last block");
        heapFull = true;
              
      /* This is NOT the last block, update fL*/
      } else {
        printf("%s\n", "1st block & NOT last block");
        freelist = freelist->next;
        freelist->prev = NULL;
        // fL_ptr_found->next = NULL;
      }

    /* Some block other than first block is being filled */
    } else {
      /* This is the last block */
      if (fL_ptr_found->next == NULL) {
        printf("%s\n", "NOT 1st block & last block");
        fL_ptr_found->prev->next = NULL;
      /* This is NOT the last block */
      } else {
        printf("%s\n", "NOT 1st block & NOT last block");
        fL_ptr_found->prev->next = fL_ptr_found->next;
        fL_ptr_found->next->prev = fL_ptr_found->prev;
        fL_ptr_found->next = NULL;
        fL_ptr_found->prev = NULL;
      }

      /* Update freelist*/
      metadata_t* temp_ptr = fL_ptr_found;
      while(temp_ptr->prev != NULL) {
        temp_ptr = temp_ptr->prev;
      }
      freelist = temp_ptr;
    }


  /* Malloced size is smaller than found block */
  /* Potentially split! */  
  } else {
    /* Malloc size is at least 2 headers sizes smaller than found block */
    if (fL_ptr_found->size >= (METADATA_T_ALIGNED + total_Block_Size)) {
      if (freelist == NULL) {
        printf("%s\n\n", "ERROR: FREELIST IS NULL!!!!!!!!! (this shouldn't happen)");
      }
      printf("%s\n", "splitting, malloc doesn't fit exactly");
      /* Create new header */
      metadata_t* new_header = (void*) fL_ptr_found + total_Block_Size;
      /* Determine size of new split block */
      new_header->size = fL_ptr_found->size - total_Block_Size;    
      /* Updated size of the malloced block*/
      fL_ptr_found->size = total_Block_Size;
      printf("%s%zu%s%zu\n", "new header size: ", new_header->size, ", updated found block's size: ", fL_ptr_found->size);

      // /* Find block right before and after malloced block */
      // metadata_t* prev_fL_block = freelist;
      // while ((prev_fL_block->next != NULL) && (prev_fL_block->next < fL_ptr_found)) {
      //   prev_fL_block = prev_fL_block->next;
      // }
      // metadata_t* next_fL_block = prev_fL_block->next; 

    
      /* Found block is first block and last block */
      if ((fL_ptr_found->prev == NULL) && (fL_ptr_found->next == NULL)) {
        printf("%s\n", "found block is first block and last block");
        new_header->next = NULL;
        new_header->prev = NULL;
        freelist = new_header;
        fL_ptr_found->next = NULL;
        fL_ptr_found->prev = NULL;

      /* Found block is first block, but there are other blocks afterwards */
      } else if ((fL_ptr_found->prev == NULL) && (fL_ptr_found->next != NULL)) {
        printf("%s\n", "found block is first block, other blocks after");
        fL_ptr_found->next->prev = new_header;
        new_header->next = fL_ptr_found->next;
        new_header->prev = NULL;
        fL_ptr_found->next = NULL;
        fL_ptr_found->prev = NULL;

      /* Found block is last block, but there are other blocks ahead */
      } else if ((fL_ptr_found->next == NULL) && (fL_ptr_found->prev != NULL)) {
        printf("%s\n", "found block is last block, other blocks before");
        fL_ptr_found->prev->next = new_header;
        new_header->prev = fL_ptr_found->prev;
        new_header->next = NULL;
        fL_ptr_found->next = NULL;
        fL_ptr_found->prev = NULL;

      /* Found block is in between two other free blocks */
      } else  {
        printf("%s\n", "found block is in between two other free blocks");
        fL_ptr_found->prev->next = new_header;
        fL_ptr_found->next->prev = new_header;
        new_header->next = fL_ptr_found->next;
        new_header->prev = fL_ptr_found->prev;
        fL_ptr_found->next = NULL;
        fL_ptr_found->prev = NULL;
      }

      // /* Malloced block no longer points to anything*/
      // fL_ptr_found->next = NULL;
      // fL_ptr_found->prev = NULL;
      // printf("%s\n", "found block no longer points to anything");

      /* Update freelist */
      // metadata_t* temp_ptr2 = new_header;
      // while(temp_ptr2->prev != NULL) {
      //   temp_ptr2 = temp_ptr2->prev;
      //   // printf("%s\n", "updating freelist");
      // }
      // printf("%s\n", "get here 1");
      // freelist = temp_ptr2;

      // /* Updated size of the malloced block*/
      // fL_ptr_found->size = total_Block_Size;
      // printf("%s%zu\n", "updated header size: ", fL_ptr_found->size); 



    /* Don't split */
    } else {
      printf("%s\n", "don't split!");
      
      /* Found block is first block and last block */
      if ((fL_ptr_found->prev == NULL) && (fL_ptr_found->next == NULL)) {
        printf("%s\n", "found block is first block and last block");
        assert(freelist = fL_ptr_found);
        freelist = NULL;

      /* Found block is first block, but there are other blocks afterwards */
      } else if ((fL_ptr_found->prev == NULL) && (fL_ptr_found->next != NULL)) {
        printf("%s\n", "found block is first block, other blocks after");
        assert(freelist = fL_ptr_found);
        freelist = freelist->next;
        fL_ptr_found->next = NULL;

      /* Found block is last block, but there are other blocks ahead */
      } else if ((fL_ptr_found->next == NULL) && (fL_ptr_found->prev != NULL)) {
        printf("%s\n", "found block is last block, other blocks before");
        fL_ptr_found->prev->next = NULL;
        fL_ptr_found->prev = NULL;

      /* Found block is in between two other free blocks */
      } else {
        printf("%s\n", "found block is in between two other free blocks");
        fL_ptr_found->prev->next = fL_ptr_found->next;
        fL_ptr_found->next->prev = fL_ptr_found->prev;
        fL_ptr_found->next = NULL;
        fL_ptr_found->prev = NULL;
      }
    }     
  }
  printf("%s\n\n", "END DMALLOC");

  return (void*) fL_ptr_found + METADATA_T_ALIGNED;
}

void dfree(void* ptr) {
  /* your code here */
  metadata_t* ptr_free = (void*) ptr - METADATA_T_ALIGNED;

  if (heapFull) {
      printf("%s\n", "heap is completely full");
      /* After dfree, heap should no longer be full */
      heapFull = false;
      freelist = ptr_free;
      freelist->next = NULL;
      freelist->prev = NULL;
      return;
  }
  /* Freed block becomes first block in freelist */
  if (ptr_free < freelist){
      printf("%s\n", "freed block becomes first block");

    /* Freed block is block exactly before freelist */
    /* Coalesce! */
    if ((void *) ptr_free + ptr_free->size == (void*) freelist) {
      printf("%s\n", "freed block is exactly before freelist");
      ptr_free->size = ptr_free->size + freelist->size;
      ptr_free->next = freelist->next;
      ptr_free->prev = NULL;

      if (freelist->next != NULL) {
        freelist->next->prev = ptr_free;
      }
      /* Update freelist */
      freelist = ptr_free;

    /* Freed block is not exactly before freelist */
    } else { 
      printf("%s\n", "freed block is NOT exactly before freelist");
      ptr_free->next = freelist;
      ptr_free->prev = NULL;
      freelist->prev = ptr_free;
      freelist = ptr_free;
    }

  /* Freed block comes after freelist (is not first block in freelist) */
  } else {
    /* Find the blocks before and after the Freed block */
    metadata_t* prev_ptr_free = freelist;
    printf("%s\n", "freed block comes after freelist");
    while ((prev_ptr_free->next < ptr_free) && (prev_ptr_free->next != NULL)) {
      prev_ptr_free = prev_ptr_free->next;
    }
    metadata_t* next_ptr_free = prev_ptr_free->next;

    /* Freed block is last block */
    if (prev_ptr_free->next == NULL) {
    printf("%s\n", "freed block becomes last block in freelist");
      /* Previous block is free */
      if ((void*) prev_ptr_free + prev_ptr_free->size == (void*) ptr_free) {
        printf("%s\n", "previous block is free");
        prev_ptr_free->size = prev_ptr_free->size + ptr_free->size;
      /* Previous block is NOT free*/
      } else {
        printf("%s\n", "previous block is NOT free");
        ptr_free->prev = prev_ptr_free;
        ptr_free->next = NULL;
        prev_ptr_free->next = ptr_free;
      }

    /* Freed block is somewhere in middle */
    } else {
      printf("%s\n", "freed block is in middle");
      assert(next_ptr_free != NULL );
      /* Check prev and next blocks to see if they're free */
      bool prev_block_free;
      bool next_block_free;
      if ((void*) ptr_free == (void*) prev_ptr_free + prev_ptr_free->size) {
        prev_block_free = true;
        printf("%s\n", "prev block is free");
      } else {
        prev_block_free = false;
        printf("%s\n", "prev block is not free");
      }
      if ((void*) ptr_free + ptr_free->size == (void*) next_ptr_free) {
        next_block_free = true;
        printf("%s\n", "next block is free");
      } else {
        next_block_free = false;
        printf("%s\n", "next block not is free");
      }
      /* Four different cases for coalescing */
      /* Neither prev nor next is free */
      if ((!prev_block_free) && (!next_block_free)) {
        printf("%s\n", "neither block is free");
        prev_ptr_free->next = ptr_free;
        next_ptr_free->prev = ptr_free;
        ptr_free->next = next_ptr_free;
        ptr_free->prev = prev_ptr_free;

      /* Only prev block is free */
      } else if ((prev_block_free) && (!next_block_free)) {
        printf("%s\n", "only prev block is free");
        prev_ptr_free->size = prev_ptr_free->size + ptr_free->size;


      /* Only next block is free */
      } else if ((!prev_block_free) && (next_block_free)) {
        printf("%s\n", "only next block is free");
        prev_ptr_free->next = ptr_free;
        ptr_free->next = next_ptr_free->next;
        ptr_free->prev = prev_ptr_free;
        ptr_free->size = ptr_free->size + next_ptr_free->size;
        if (next_ptr_free->next != NULL) {
          next_ptr_free->next->prev = ptr_free;
        }
        next_ptr_free->next = NULL;
        next_ptr_free->prev = NULL;

      /* Both prev & next block are free */
      } else {
        printf("%s\n", "both prev & next block are free");
        prev_ptr_free->size = prev_ptr_free->size + ptr_free->size + next_ptr_free->size;
        /* Next block is the last block in freelist */
        if (next_ptr_free->next == NULL) {
          prev_ptr_free->next = NULL;

        /* Next block is NOT the last block in freelist */
        } else {
          next_ptr_free->next->prev = prev_ptr_free;
          prev_ptr_free->next = next_ptr_free->next;
        }
        ptr_free->next = NULL;
        ptr_free->prev = NULL;
        next_ptr_free->prev = NULL;
        next_ptr_free->next = NULL;
      }
      // return;
    }

  }
  printf("%s\n", "END DFREE");
  printf("\n");
  return;
}

bool dmalloc_init() {

  /* Two choices: 
   * 1. Append prologue and epilogue blocks to the start and the
   * end of the freelist 
   *
   * 2. Initialize freelist pointers to NULL
   *
   * Note: We provide the code for 2. Using 1 will help you to tackle the 
   * corner cases succinctly.
   */

  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);
  /* returns heap_region, which is initialized to freelist */
  freelist = (metadata_t*) sbrk(max_bytes); 
  /* Q: Why casting is used? i.e., why (void*)-1? */
  /* A: Used to test if sbrk failed */
  if (freelist == (void *)-1)
    return false;
  freelist->next = NULL;
  freelist->prev = NULL;
  freelist->size = max_bytes-METADATA_T_ALIGNED;
  return true;
}

/* for debugging; can be turned off through -NDEBUG flag*/
void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL) {
    printf("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p\t",
	  freelist_head->size,
	  freelist_head,
	  freelist_head->prev,
	  freelist_head->next);
    freelist_head = freelist_head->next;
    printf("\n");  
  }
  printf("\n");
}
