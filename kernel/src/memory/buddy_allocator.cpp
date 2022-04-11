#include <memory/buddy_allocator.hpp>

#include <stddef.h>

uint8_t buddyallocatorFindOrder(size_t size, uint8_t min) {
    // this is log2(size/min)
    // TODO: check both size and min are a power of 2
    uint8_t order = 0;
    size_t num = size / min;
    while (num > 1) {
        num >>= 1; // divide by 2
        order++;
    }

    return order;
}

uint64_t alignToPowerOfTwo(uint64_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    v++;

    return v;
}

k_buddyallocator::k_buddyallocator(virtual_address_t base, size_t size, uint8_t minBlockSize) {
    this->maxOrder = buddyallocatorFindOrder(size, minBlockSize);
    this->minBlockSize = minBlockSize;
    this->base = base;
    this->size = size;

    // Reset all the lists
    for (uint8_t i = 0; i <= this->maxOrder; i++)
        this->avail[i].linkf = this->avail[i].linkb = &this->avail[i]; 

    // Put the entire memory block in the topmost entry of the list
    k_buddyallocator_block_hdr *baseBlock = (k_buddyallocator_block_hdr *)base;
    this->avail[this->maxOrder].linkf = baseBlock;
    baseBlock->linkb = &this->avail[this->maxOrder];
    this->avail[this->maxOrder].linkb = baseBlock;
    baseBlock->linkf = &this->avail[this->maxOrder];
    baseBlock->tag = 1;
    baseBlock->kval = this->maxOrder;
}

virtual_address_t k_buddyallocator::allocate(size_t size) {
    // Find the minimum order for this allocation
    uint8_t requiredOrder = buddyallocatorFindOrder(alignToPowerOfTwo(size + sizeof(k_buddyallocator_block_hdr)), this->minBlockSize);

    // Find the smallest order available, able to perform the allocation
    uint8_t j;
    for (j = requiredOrder; j <= this->maxOrder; j++)
        if (!LINKED_LIST_IS_EMPTY(&this->avail[j])) break;
    
    // Nothing was found, there isn't enough memory or too much fragmented
    if (j > this->maxOrder) return NULL;

    // Remove the first block from the available list
    k_buddyallocator_block_hdr *block = (&this->avail[j])->linkf; // Get the first block
    k_buddyallocator_block_hdr *nextBlock = block->linkf; // the next block of the same order
    this->avail[j].linkf = nextBlock;
    nextBlock->linkb = &this->avail[j];
    
    // Do we need to split the block?
    while ((j--) > requiredOrder) {
        // Split the block
        nextBlock = (k_buddyallocator_block_hdr *)((uint64_t)block + (1L << j)); // block + 2^j 
        nextBlock->tag = 1; // it is available
        nextBlock->kval = j;
        nextBlock->linkf = nextBlock->linkb = &this->avail[j]; // put the block in the lower-order list
        this->avail[j].linkf = this->avail[j].linkb = nextBlock;
    }

    block->tag = 0;
    block->kval = requiredOrder;
    return (virtual_address_t)((uint64_t)block + sizeof(k_buddyallocator_block_hdr));
}

void k_buddyallocator::deallocate(virtual_address_t addr) {
    // Calculate the block header address
    k_buddyallocator_block_hdr *block = (k_buddyallocator_block_hdr *)(addr - sizeof(k_buddyallocator_block_hdr));

    // Some validation is needed
    if (((virtual_address_t)block < this->base) || ((virtual_address_t)block > (this->base + size))) return; // Is the block even in our range
    if (block->tag == 1) return; // Block is already available, no need to free it
      
    // Start merging the blocks recursivly from the addr and up.
    k_buddyallocator_block_hdr *buddy = BUDDYALLOCATOR_GET_BUDDY(block);
    while ((block->kval != this->maxOrder) && (buddy->tag == 1) && (buddy->kval == block->kval)) {
        // Remove the buddy from it's current list, it need to go up one
        buddy->linkb->linkf = buddy->linkf;
        buddy->linkf->linkb = buddy->linkb;
        
        // We always want to address the "left buddy"
        if ((virtual_address_t)buddy < (virtual_address_t)block) block = buddy;

        block->kval++;
        buddy = BUDDYALLOCATOR_GET_BUDDY(block);
    } 

    block->tag = 1;
    // Put it on the list according to it's kval
    k_buddyallocator_block_hdr *firstBlock = this->avail[block->kval].linkf;
    block->linkf = firstBlock;
    firstBlock->linkb = block;
    block->linkb = &this->avail[block->kval];
    this->avail[block->kval].linkf = block;
}