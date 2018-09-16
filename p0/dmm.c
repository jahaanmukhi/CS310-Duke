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
  bool isFree;
} metadata_t;

/* freelist maintains all the blocks which are not in use; freelist is kept
 * sorted to improve coalescing efficiency 
 */

static metadata_t* freelist = NULL;
// bool heapFull = false;


void* dmalloc(size_t numbytes) {
  printf("%s\n", "START DMALLOC");

  /* initialize through sbrk call first time */
  if(freelist == NULL) { 		
    if(!dmalloc_init()) 
      return NULL;
  }
  assert(numbytes > 0);
  /* your code here */
  /* Check to make sure heap isn't full */
  // if (heapFull) {
  //   printf("%s\n", "ERROR: Heap is full");  
  //   return NULL;
  // }
  size_t total_Block_Size = METADATA_T_ALIGNED + ALIGN(numbytes);  
  size_t align_numbytes = ALIGN(numbytes);

  printf("%s%zu\n", "total_Block_Size: ", total_Block_Size);
  metadata_t* fL_ptr = freelist;
  metadata_t* fL_ptr_found;
  while (fL_ptr) {
    if ((fL_ptr->size < align_numbytes) || !(fL_ptr->isFree)) {
      fL_ptr = fL_ptr->next;
    }
    // printf("%zu\n", fL_ptr->size);
    else {
      break;
    }
  }

  if (!fL_ptr) {
    /* This is the last block, no block is big enough */
    printf("%s\n\n", "no block is big enough"); 
    return NULL;
  } else {
    fL_ptr_found = fL_ptr;
  }




  printf("%s%zu%s%d\n", "found block size ", fL_ptr_found->size, " at address ", fL_ptr_found);
  /* The malloced size exactly fits the found block */
  if (fL_ptr_found->size == align_numbytes) {
    printf("%s\n", "malloc fits exactly"); 
    fL_ptr_found->isFree = false;
  /* Malloced size is smaller than found block */
  /* Potentially split */  
  } else {
    /* Malloc size is at least 2 headers sizes smaller than found block */
    /* Split! */
    if (fL_ptr_found->size >= (METADATA_T_ALIGNED + align_numbytes)) {
      printf("%s\n", "split block");
      fL_ptr_found->isFree = false;
      /* Create new header */
      metadata_t* new_header = (void*) fL_ptr_found + total_Block_Size;
      /* Determine size of new split block */
      new_header->size = fL_ptr_found->size - align_numbytes - METADATA_T_ALIGNED;
      printf("%s%zu\n", "new header size: ", new_header->size);
      /* Set new block to be free */
      new_header->isFree = true;    
      /* Update size of the malloced block */
      fL_ptr_found->size = align_numbytes;

      /* Update free list */
      /* Exists another block after found block*/
      if (fL_ptr_found->next) {
        new_header->next = fL_ptr_found->next;
        new_header->prev = fL_ptr_found;
        fL_ptr_found->next->prev = new_header;
        fL_ptr_found->next = new_header;
      /* Found block is last block */
      } else {
        fL_ptr_found->next = new_header;
        new_header->next = NULL;
        new_header->prev = fL_ptr_found;
      }
    /* Don't split! */
    } else {
      printf("%s\n", "don't split block");
      fL_ptr_found->isFree = false;
    }
  }
  /* Check to see if the heap is full */

  metadata_t* temp_ptr = freelist;
  size_t sum = 0;
  size_t max_size = 0;
  metadata_t* address;
  while (temp_ptr) {
    // printf("%p\n", temp_ptr);
    if (temp_ptr->isFree) {
      sum = sum + temp_ptr->size;
      if (temp_ptr->size > max_size) {
       max_size = temp_ptr->size;
       address = temp_ptr;
      }
    }
    temp_ptr = temp_ptr->next;
  }

  printf("%s%zu\n", "Amount of free memory after dmalloc : ", sum);
  printf("%s%zu%s%d\n", "Biggest free block: ", max_size, " at address ", address);

  printf("%s\n\n", "END DMALLOC");
  // print_freelist();
  return (void*) fL_ptr_found + METADATA_T_ALIGNED;
}

void dfree(void* ptr) {
  /* your code here */
  printf("%s\n", "START DFREE");
  metadata_t* ptr_free = (void*) (ptr - METADATA_T_ALIGNED);
  printf("%s%d%s%zu%s\n", "free this address: ", ptr_free, " for ", ptr_free->size, " bytes");

  /* Trying to free something that was never malloced or freelist is empty */
  if (!(ptr_free) || !(freelist)) {
    printf("%s\n", "ERROR: CAN'T FREE SOMETHING THAT DOESN'T EXIST");
    return;
  }

  /* First and only block */
  if (!(ptr_free->prev) && !(ptr_free->next)) {
    printf("%s\n", "freed block is first and only block");
    /* Freed block should be first block of freelist */
    /* No coalescing is possible */
    ptr_free->isFree = true;
    ptr_free->size = ptr_free->size;

  /* First block, more blocks after */
  } else if (!(ptr_free->prev) && (ptr_free->next)) {
    printf("%s\n", "freed block is first block, more blocks after");
    /* Next block is free, coalesce */
    if (ptr_free->next->isFree) {
      printf("%s\n", "next is free, coalesce");
      
      /* Exists next next block */
      if (ptr_free->next->next) {
        /* Update size */
        ptr_free->size = ptr_free->size + ptr_free->next->size + METADATA_T_ALIGNED; 
        /* Set block to be free */
        ptr_free->isFree = true;
        /* Adjust freelist pointers */
        metadata_t* a1 = (metadata_t*) ptr_free;
        // metadata_t* a2 = (metadata_t*) ptr_free->next;
        metadata_t* a3 = (void*) ptr_free->next->next;
        ptr_free->next = a3;
        ptr_free->next->next->prev = a1;
        // ptr_free->next->next = NULL;
        // ptr_free->next->prev = NULL;

      /* Not Exist next next block */
      } else {
        /* Update size */
        ptr_free->size = ptr_free->size + ptr_free->next->size + METADATA_T_ALIGNED; 
        /* Set block to be free */
        ptr_free->isFree = true;
        // ptr_free->next->isFree = true;
        /* Adjust freelist pointers */
        // ptr_free->next->prev = NULL;
        ptr_free->next = NULL;
      }

    /* Next block isn't free, don't coalesce */
    } else {
      printf("%s\n", "next not free, no coalesce");
      ptr_free->isFree = true;
      ptr_free->size = ptr_free->size; 
    }

  /* Last block, more blocks before */
  } else if (!(ptr_free->next) && (ptr_free->prev)) {
    printf("%s\n", "freed block is last block, more blocks before");
    /* Previous block is free, coalesce */
    if (ptr_free->prev->isFree) {
      printf("%s\n", "prev block is free, coalesce");
      /* Update size */
      ptr_free->prev->size = ptr_free->prev->size + ptr_free->size + METADATA_T_ALIGNED;
      /* Set block to be free */
      ptr_free->prev->isFree = true;
      // ptr_free->isFree = true;
      /* Adjust freelist pointers */
      ptr_free->prev->next = NULL;
      // ptr_free->next = NULL;
      // ptr_free->prev = NULL;
      
    /* Previous block is not free */
    /* Don't coalesce */
    } else {
      printf("%s\n", "prev not free, no coalesce");
      ptr_free->isFree = true;
      ptr_free->size = ptr_free->size;
    }

  /* Middle block */
  } else {
    printf("%s\n", "freed block is a middle block");
    /* Case 1: Both prev and next blocks free, coalesce both */
    if ((ptr_free->next->isFree) && (ptr_free->prev->isFree)) {
      printf("%s\n", "both prev and next are free, coalesce both");
      /* Exists next next block */
      if (ptr_free->next->next) {
        /* Update size */
        ptr_free->prev->size = ptr_free->prev->size + ptr_free->size + ptr_free->next->size + 2 * METADATA_T_ALIGNED;
        /* Set blocks to be free */
        ptr_free->prev->isFree = true;
        // ptr_free->next->isFree = true;
        // ptr_free->isFree = true;
        /* Adjust freelist pointers */
        metadata_t* ad1 = (void*) ptr_free->next->next;
        metadata_t* ad2 = (void*) ptr_free->prev;
        ptr_free->prev->next = ad1;
        ptr_free->next->next->prev = ad2;
        // ptr_free->next->next = NULL;
        // ptr_free->next->prev = NULL;
        // ptr_free->prev = NULL;
        // ptr_free->next = NULL; 

      /* Not Exist next next block */
      } else {
        /* Update size */
        ptr_free->prev->size = ptr_free->prev->size + ptr_free->size + ptr_free->next->size + 2 * METADATA_T_ALIGNED;
        /* Set blocks to be free */
        ptr_free->prev->isFree = true;
        // ptr_free->next->isFree = true;
        // ptr_free->isFree = true;
        /* Adjust freelist pointers */
        ptr_free->prev->next = NULL;
        // ptr_free->next->prev = NULL;
        // ptr_free->next = NULL;
        // ptr_free->prev = NULL;
      }

    /* Case 2: Only next block is free, coalesce once */
    } else if ((ptr_free->next->isFree) && (!ptr_free->prev->isFree)) {
      printf("%s\n", "next is free, coalesce");
      /* Exists next next block */
      if (ptr_free->next->next) { 
        printf("%s\n", "exists next next block");
        /* Update size */
        ptr_free->size = ptr_free->size + ptr_free->next->size + METADATA_T_ALIGNED;
        /* Set block to be free */
        ptr_free->isFree = true;
        // ptr_free->next->isFree = true;
        /* Adjust freelist pointers */
        metadata_t* addr1 = (metadata_t*) (void*) ptr_free;
        metadata_t* addr2 = (metadata_t*) (void*) ptr_free->next->next;
        metadata_t* block_a1 = ptr_free->next->next;

        block_a1->prev = addr1;
        ptr_free->next = addr2;
        // ptr_free->next->next = NULL;
        // ptr_free->next->prev = NULL;

      /* Not Exist next next block */
      } else {
        printf("%s\n", "not exist next next block");
        /* Update size */
        ptr_free->size = ptr_free->size + ptr_free->next->size + METADATA_T_ALIGNED;
        /* Set block to be free */
        ptr_free->isFree = true;
        // ptr_free->next->isFree = true;
        /* Adjust freelist pointers */
        ptr_free->next = NULL;
        // ptr_free->next->prev = NULL;
      }

    /* Case 3: Only prev block is free, coalesce once */
    } else if ((!ptr_free->next->isFree) && (ptr_free->prev->isFree)) {
      printf("%s\n", "prev is free, coalesce");
      /* Update size */
      ptr_free->prev->size = ptr_free->prev->size + ptr_free->size + METADATA_T_ALIGNED;
      /* Set block to be free */
      ptr_free->prev->isFree = true;
      // ptr_free->isFree = true;
      /* Adjust freelist pointers */
      metadata_t* adr1 = (void*) ptr_free->next;
      metadata_t* adr2 = (void*) ptr_free->prev;
      ptr_free->prev->next = adr1;
      ptr_free->next->prev = adr2;
      // ptr_free->next = NULL;
      // ptr_free->prev = NULL;

    /* Case 4: Neither prev nor next blocks are free, no coalesce */  
    } else {
      printf("%s\n", "neither prev nor next are free, no coalesce");
      ptr_free->isFree = true;
      ptr_free->size = ptr_free->size;
    }
  }
  metadata_t* temp_ptr = freelist;
  size_t sum = 0;
  size_t max_size = 0;
  metadata_t* addrs;
  while (temp_ptr != NULL) {
    if (temp_ptr->isFree) {
      sum = sum + temp_ptr->size;
      if (temp_ptr->size > max_size) {
        max_size = temp_ptr->size;
        addrs = temp_ptr;
      }
    }
    temp_ptr = temp_ptr->next;
  }
  printf("%s%zu\n", "Amount of free memory after dfree: ", sum);
  printf("%s%zu%s%d\n", "Biggest free block: ", max_size, " at address ", addrs);
  printf("%s\n", "END DFREE");
  printf("\n");
  // print_freelist();
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
  if (freelist == (void *)-1)
    return false;
  freelist->next = NULL;
  freelist->prev = NULL;
  freelist->size = max_bytes-METADATA_T_ALIGNED;
  freelist->isFree = true;
  return true;
}

/* for debugging; can be turned off through -NDEBUG flag*/
void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL) {
    printf("\tFreelist Size:%zd, Head:%d, Prev:%d, Next:%d\t, isFree:%d\n",
	  freelist_head->size,
	  freelist_head,
	  freelist_head->prev,
	  freelist_head->next,
    freelist_head->isFree);
    freelist_head = freelist_head->next;
  }
  printf("\n");
}