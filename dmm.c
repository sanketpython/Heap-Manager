#include <stdio.h>  
#include <unistd.h> 
#include <assert.h> 
#include "dmm.h"

/* Metadata stores info about heap blocks not available to the user.  It is for the heap manager to maintain which blocks 
are free and how many free bytes they have */
typedef struct metadata {
  size_t size;
  bool freed; 
  struct metadata* next;
  struct metadata* prev; 
} metadata_t;

/*The free list preserves which blocks are in use/not in use*/
static metadata_t* freelist = NULL;
metadata_t * head = NULL; 
metadata_t * endlist = NULL;
int s = 440; 
char *end;

metadata_t* split (metadata_t * curr, int numbytes){
  size_t size = sizeof(metadata_t) + numbytes;
  /*If the current block's size minus the size needed is less than the size of metadata, return the pointer. 
    This prevents a split that would result in a block that is smaller than a metadata, which could never be
    allocated. */
  if((curr->size - size) <= sizeof(metadata_t)){
    return curr; 
  }
  metadata_t* header1 = curr; //Initializing header for new block to be returned
  size_t oldsize = curr->size; 
  metadata_t* next = curr->next; 
  curr = curr +1; //Move pointer past the header 
  char *ptr = (char*) curr; 
  ptr = ptr + numbytes; 

  curr = (metadata_t*)ptr; //Restore pointer to metadata type
  /*Initialize new free block and block to be returned*/
  metadata_t* header2 = curr; //Create header for leftover block
  header2->size = oldsize - (numbytes + sizeof(metadata_t)); 
  header1->size = oldsize - header2->size; 
  header1->next = header2; 
  header2->prev = header1; 

  if(next !=NULL){
    next->prev = header2; 
  }
  header2->next = next; 
  header1->freed = false; 
  header2->freed = true; 
  return header1; 

}

/* Allocate memory region for desired number of bytes */
void* dmalloc(size_t numbytes) { 
  
  if(freelist == NULL) { 			
    if(!dmalloc_init()) //Initializes heap region through sbrk call
      return NULL;
  }

  assert(numbytes > 0);
  numbytes = ALIGN(numbytes); //Align desired number of bytes according to wordsize = 8 bytes
  metadata_t* curr = freelist; 

  while(curr !=NULL){
    /*If the size of the block is equal to what is needed, set it to free and return the pointer */ 
    if(curr->size == (numbytes + sizeof(metadata_t)) && curr->freed){
      curr->freed = false;
      return (void*)(curr +1); //Point to byte after metdata
    }
    /*If the size of the block is greater than what is needed, split it, set that block to free, and return the pointer */
    if((sizeof(metadata_t) + numbytes) < curr->size  && curr->freed){
      curr = split(curr, numbytes); 
      curr->freed = false;
      return (void*)(curr +1); //Points to byte after metadata
    }
  curr = curr->next; 
  }
  return NULL;
  }


void coalesce(metadata_t * curr){
  int totalsize; 

/*If at the front of the list, check if the next block is free, then coalesce */
  if(curr == head){
    if(curr->next->freed){
      totalsize = curr->size + curr->next->size; 
      curr->next = curr->next->next; 
      if(curr->next != NULL){
        curr->next->prev = curr; 
      }
      curr->size = totalsize; 
    }
    return; 
  }
  /*If at the end of the list, check if the previous block is free, then coalesce */
  if(curr == endlist){
    if(curr->prev->freed){
      totalsize = curr->size + curr->prev->size; 
      curr = curr->prev; 
      curr->next = NULL; 
      curr->size = totalsize; 
    }
    return;
  }
/*If in the middle of the list, check if the previous and next block are free, then coalesce*/
  if(curr->prev != NULL){
      if(curr->prev->freed){
        totalsize = curr->size + curr->prev->size; 
        curr = curr->prev; 
        curr->next = curr->next->next; 
        if(curr->next != NULL){
          curr->next->prev = curr; 
        }
      curr->size = totalsize; 
      }
  }

  if(curr->next != NULL){
    if(curr->next->freed){
      totalsize = curr->size + curr->next->size;
      curr->next = curr->next->next; 
      if(curr->next != NULL){
        curr->next->prev = curr; 
      }
      curr->size = totalsize; 
    }
  }
  return;
}


void dfree(void* ptr) {
  if(ptr == NULL){
    return;
  }
  head = freelist; 
  /*Move back the number of bytes for a metadata to point to beginning of a metadata block and iterate over freelist */
  ptr = ptr - sizeof(metadata_t); 
  metadata_t * curr = freelist; 

  while(curr != NULL){
      /*If the current block pointer equals the desired pointer and it is in use, set it free*/
    if(ptr == curr && !curr->freed){ 
      curr->freed = true;
      coalesce(curr); //Coalesce to create bigger free blocks for later dmalloc use 
      return;
    }
    curr = curr->next; 
  }
  return;
}

bool dmalloc_init() {
  size_t max_bytes = ALIGN(MAX_HEAP_SIZE); //Set the max heap size, and align it to the word size
  freelist = (metadata_t*) sbrk(max_bytes); //Return the heap region and intializes the freelist
  end = (char*) freelist; 
  end = end + max_bytes; 
  endlist = (metadata_t*)end; //Store pointer to the end of the list

  if (freelist == (void *)-1)
    return false;
  freelist->next = NULL;
  freelist->prev = NULL;
  freelist->freed = true; 
  freelist->size = max_bytes-METADATA_T_ALIGNED;
  return true;
}

//debugging purposes
void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL) {
    DEBUG("\tFreelist Size: %zd, Head: %p, Prev: %p, Next: %p, Freed: %d\t",
	  freelist_head->size,
	  freelist_head,
	  freelist_head->prev,
	  freelist_head->next,
    freelist_head->freed);
    freelist_head = freelist_head->next;
  }
  DEBUG("\n");
}

