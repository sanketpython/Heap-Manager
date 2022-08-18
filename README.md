
## How It Works<br/>
Upon initialization, the dmalloc function calls sbrk, which carves out a portion of virtual memory. The pointer returned by that function represents the virtual heap region. It also represents the pointer to the "freelist".

### The Freelist <br/>
The freelist is a doubly linked list that maintains the state of the heap. It contains "metadata" headers that act as borders for the heap blocks. These headers contain data about the size of the block, whether it is in use/not in use, and pointers to the previous and next blocks in the free list. Dmalloc and dfree manipulate the freelist to allocate/deallocate memory. 

### Dmalloc <br/>
Dmalloc returns a pointer to the heap block with the required number of bytes. First, it resets the required size equal to the size of the header plus the number of bytes needed. Then, it iterates over the freelist, checking headers for a size that is greater or equal to the new required size. If the block is of equal size, then perfect! Dmalloc returns the pointer to that block. If ihe block is of greater size, then dmalloc splits the block. In the split function, it sets two headers: the first being the block of the desired size, and the second being the block that represents the rest of the unused space. The pointer to the first block is then returned back to dmalloc. 

### Dfree<br/>
Dfree frees the block with the pointer passed into it. It first resets the pointer to the pointer minus the header size. Then, it iterates over the freelist and checks if the pointers are equal. Once the desired block is found, it is set to free and coalesced with blocks that are also free. The purpose of the coalesce function is to prevent memory fragmentation. Essentially, it frees up more space between blocks so larger regions of memory can be dmalloced in the future. In the coalesce function, it checks the previous and next blocks and unlinks them from the freelist if they are not in use. 

The 'dmm.h' file is important for configuring the project. For example, it sets the word size for alignment in ALIGN, and the maximum size of the heap region in sbrk. 
These tests proved my 'dmm.c' accurately implemented the heap manager with optimized memory utilization. 

