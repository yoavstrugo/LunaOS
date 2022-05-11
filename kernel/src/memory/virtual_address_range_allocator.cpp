#include <memory/virtual_address_range_allocator.hpp>

#include <memory/heap.hpp>
#include <memory/paging.hpp>
#include <logger/logger.hpp>
#include <stddef.h>

k_virtual_address_range_allocator::k_virtual_address_range_allocator() {
    this->head = 0;
}

void k_virtual_address_range_allocator::addRange(virtual_address_t start, virtual_address_t end) {
    // TODO: check that start and end are page aligned
    k_address_range_header *addressHeader = new k_address_range_header;

    addressHeader->base = start;
    addressHeader->pages = (end - start) / PAGE_SIZE;
    addressHeader->used = false;

    if(!this->head) {
        this->head = addressHeader;
        return;
    } 

    // We need to sort the list, so we may be able to merge ranges
    k_address_range_header *found = 0;
    k_address_range_header *curr = this->head;

    while (curr) {
        if (addressHeader->base < curr->base)
            found = curr;
        curr = curr->next;
    }

    if (found) {
        // Put it in between the lower range and higher range
        addressHeader->next = found->next;
        found->next = addressHeader;
    } else {
        // It's the smallest in the list, it shall be first
        addressHeader->next = this->head->next;
        this->head = addressHeader;
    }

    // TODO: merge ranges
}

virtual_address_t k_virtual_address_range_allocator::allocateRange(uint64_t pages) {
    k_address_range_header *range = this->head;
    
    // Find a range to fit the allocation
    while (range) {
        if (!range->used && range->pages >= pages)
            break;

        range = range->next;
    }

    if (!range) {
        logDebugn("%! Couldn't find any virtual ranges to fit allocation,", "[Virtual Address Range Allocator]");
        return NULL;
    }

    range->used = true;
    range->pages = pages;

    if (range->pages > pages) {
        k_address_range_header *split = new k_address_range_header;
        split->used = false;
        split->next = range->next;
        split->pages = range->pages - pages;
        split->base = range->base + pages * PAGE_SIZE;

        range->next = split;
    }

    return range->base;
}