

// // #include <memory/Paging.hpp>

// PageManager::PageManager(PhysicalMemoryAllocator *physicalMemoryAllocator) {
//     this->_physicalMemoryAllocator = physicalMemoryAllocator;

//     // Create the PML4
//     // physical_address PML4addr = this->_physicalMemoryAllocator->allocatePage();
//     // memset((char *)PML4addr, 0, PAGE_SIZE); // dedicate a full page for the table

//     // For now get the PML4 created by the bootloader
//     asm volatile("mov %%cr3, %0" : "=r"(this->_PML4));
// }

// void PageManager::mapPage(linear_address pageAddress, physical_address physicalAddress) {
//     PageTableEntry PML4e, PDPe, PDe, PTe;
//     PageTable *PDP, *PD, *PT;

//     // Look at PML4
//     PML4e = this->_PML4->entries[PML4_indexer(pageAddress)];
//     if (!PML4e.present) {
//         // PDP is not present, lets create one
//         PDP = (PageTable *)this->_physicalMemoryAllocator->allocatePage();
//         memset((char *)PDP, 0, PAGE_SIZE);

//         // Modify properties
//         PML4e.address = (physical_address)PDP >> 12;
//         PML4e.present = true;
//         PML4e.readWrite = true;

//         this->_PML4->entries[PML4_indexer(pageAddress)] = PML4e;
//     } else {
//         PDP = (PageTable *)((physical_address)PML4e.address << 12);
//     }

//     // Look at PDP
//     PDPe = PDP->entries[PDP_indexer(pageAddress)];
//     if (!PDPe.present) {
//         // PD is not present, lets create one
//         PD = (PageTable *)this->_physicalMemoryAllocator->allocatePage();
//         memset((char *)PD, 0, PAGE_SIZE);

//         // Modify properties
//         PDPe.address = (physical_address)PD >> 12;
//         PDPe.present = true;
//         PDPe.readWrite = true;

//         PDP->entries[PDP_indexer(pageAddress)] = PDPe;
//     } else {
//         PD = (PageTable *)((physical_address)PDPe.address << 12);
//     }

//     // Look at PD
//     PDe = PD->entries[PD_indexer(pageAddress)];
//     if (!PDe.present) {
//         // PD is not present, lets create one
//         PT = (PageTable *)this->_physicalMemoryAllocator->allocatePage();
//         memset((char *)PT, 0, PAGE_SIZE);

//         // Modify properties
//         PDe.address = (physical_address)PT >> 12;
//         PDe.present = true;
//         PDe.readWrite = true;

//         PD->entries[PD_indexer(pageAddress)] = PDe;
//     } else {
//         PT = (PageTable *)((physical_address)PDe.address << 12);
//     }

//     PTe = PT->entries[PT_indexer(pageAddress)];
//     PTe.address = physicalAddress >> 12;
//     PTe.present = true;
//     PTe.readWrite = true;
// }