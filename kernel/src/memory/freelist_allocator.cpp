#include <memory/freelist_allocator.hpp>

#include <logger/logger.hpp>

void k_freelist_allocator::init(virtual_address_t start, virtual_address_t end)
{
    this->head = (k_chunk_header *)start;
    this->head->used = false;
    this->head->size = (uint64_t)(end - start);
    this->head->next = 0;
}

void *k_freelist_allocator::allocate(uint64_t size)
{
    if (size == 0)
        return NULL;

    k_chunk_header *curr = this->head;
    while (curr)
    {
        if (!curr->used && (curr->size >= (sizeof(k_chunk_header) + size)))
        {
            k_chunk_header *split = (k_chunk_header *)((uint64_t)curr + sizeof(k_chunk_header) + size);
            split->size = curr->size - (sizeof(k_chunk_header) + size);
            split->used = false;
            split->next = curr->next;

            curr->next = split;
            curr->used = true;
            curr->size = size;

            return (void *)((uint64_t)curr + sizeof(k_chunk_header));
        }

        curr = curr->next;
    }

    return 0;
}

uint64_t k_freelist_allocator::free(void *mem)
{
    k_chunk_header *chunkHeader = (k_chunk_header *)((uint64_t)mem - sizeof(k_chunk_header));

    if (!chunkHeader->used)
        return 0; // not in use

    chunkHeader->used = false;
    uint64_t size = chunkHeader->size;

    this->mergeChunks();

    return size;
}

void k_freelist_allocator::mergeChunks()
{
    k_chunk_header *curr = this->head;
    while (curr && curr->next)
    {
        if (!curr->used && !curr->next->used)
        {
            curr->size += curr->next->size;
            curr->next = curr->next->next;
        }
        else
        {
            curr = curr->next;
        }
    }
}

void k_freelist_allocator::expand(uint64_t amount)
{
    // Get the last chunk
    k_chunk_header *tail = this->head;
    while (tail->next)
        tail = tail->next;

    k_chunk_header *newChunk = (k_chunk_header *)((uint64_t)tail + sizeof(k_chunk_header) + tail->size);
    newChunk->next = 0;
    newChunk->size = amount;
    newChunk->used = false;

    tail->next = newChunk;

    this->mergeChunks();
}