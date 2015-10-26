--------------------------------------------------------------The Allocator---------------------------------------------
        In this program an allocator that uses a free list to manage memory is implemented. A free list is simply a linked list of memory blocks that are “free” (that is, not currently allocated). The list begins as an empty list. When a call to malloc() is made when the list is empty, this program requests a chunk of memory from the OS (2MB) and adds this large block to the free list. When we exhaust this memory (after satisfying malloc() calls), the program will request more 2MB block as needed.
        When a malloc() call is processed, the program searched this free list for the first block large enough to satisfy the request, remove that block from the list, and return it. It satisfies malloc() requests in a first fit manner. To accomplish this, if a block is larger than the request size, the program splits it into two smaller blocks: one block which satisfies the request, and one which contains extra memory beyond the request size. It then re-inserts this second block into the free list. To make things simpler, requests are only satisfied in multiples of 8 bytes (It rounds the requested size up to the next 8 byte boundary).
Every block in the free list has a header at the beginning, and a footer at the end (these play an important part in the free() process, which is discussed later). The header and the footer each contains the size of the object (including the header and footer), as well as whether the block is allocated or free. In addition, the header also contains pointers to the next and previous blocks in the free list.

To allocate a block of memory, this program uses the following algorithm:
Round up the requested size to the next 8 byte boundary.
Add the size of the block’s header and footer (i.e. real_size = roundup8(requested size) + sizeof(header) + sizeof(footer)).
Traverse the free list from the beginning, and find the first block large enough to satisfy the request.
If the block is large enough to be split (that is, the remainder is strictly larger than 8 bytes plus the size of the header and footer), split the block in two. The first block (lowest memory) should be removed from the free list and returned to satisfy the request. Set the _allocated flag to true in the header/footer. The remaining block should be re-inserted into the free list.
If the block is not large enough to be split, simply remove that block from the list and return it.
If the list is empty, request a new 2MB block (plus the size of a header and footer!), insert the block into the free list, and repeat step 3.

---------------------------------------------------------Freeing Memory and Coalescing------------------------------------------------------
        In a simple world, free()ing a block of memory would involve just inserting the block back into the free list; however, over time this can create external fragmentation, where the list is divided into many small blocks which are incapable of satisfying larger requests. To combat this, this program also uses coalescing, where a block to be freed is merged with free memory immediately adjacent to the block. This way, larger blocks can be created to satisfy a broader range of requests in the future efficiently.
The header and footer play an integral part to this process. 

When freeing an object, the program checks the footer of the left neighbor (the block immediately before the object to be freed in memory) and the header of the right neighbor (the block immediately after in memory) to see if those blocks are also free. If so, then it merges one or the other with the block to be freed, and places the coalesced block back into the free list. If both the left neighbor and the right neighbor are free, it coalesces the left, the freed block and the right block into a single block . The algorithm to free a block is as follows:
Check the footer of the left neighbor. If it is free, remove it from the free list, coalesce it with the block being freed, and re-insert the new block back into the free list. Be sure to mark the object as free in the header/footer. Return.
Check the header of the right neighbor. If it is free, coalesce the two blocks and insert the new block into the free list and update the header/footer. Return.
If neither the left nor right neighbors are free, simply mark the block as free and insert it into the free list without any coalescing.
        
---------------------------------------------------------Fence Posts-----------------------------------------------------------------------------
        There is a corner case regarding coalescing that must be dealt with. If a block is either at the very beginning or the very end of the heap, accessing a header/footer beyond the boundary of a block for the purposes of coalescing may result in a crash (from accessing memory you do not own). To combat this, every time the program requests a 2MB block of memory from the OS via sbrk(), it adds an additional “dummy footer” and “dummy header” to the beginning and end of the block, respectively. These have the _allocated flag set permanently to 1, so that it warns that memory beyond these boundaries should not be coalesced. These are in addition to the meaningful header and footer that the program attaches to the block itself. So, whenever the program requests a new 2MB block from the OS, it actually requests:
 2097152 + (2 * sizeof(header)) + (2 * sizeof(footer)) 
number of bytes.

