#include <storage/ahci/ahci.hpp>

#include <logger/logger.hpp>
#include <memory/paging.hpp>
#include <memory/memory.hpp>
#include <stddef.h>
#include <kernel.hpp>

#include <strings.hpp>

k_ahci_driver::k_ahci_driver(PCICommonConfig *pciBaseAddress)
{
    this->pciBaseAddress = pciBaseAddress;
#ifdef VERBOSE_AHCI
    logDebugn("PCI BASE ADDRESS: 0x%64x", pciBaseAddress);
#endif

    physical_address_t physABAR = pciBaseAddress->u.type0.baseAddresses[5];
    virtual_address_t virtABAR = virtualAddressRangeAllocator.allocateRange(1, "ahci");
    if (virtABAR == NULL)
        kernelPanic("%! Couldn't allocated address for ABAR.", "[AHCI Driver]");

    pagingMapPage(virtABAR, physABAR);
    this->ABAR = (k_HBA_mem *)virtABAR;

    // Find how many slots each port has
    this->cmdSlots = (ABAR->cap >> 8) & 0xF0;

    // Probe all ports and look for devices
    this->portCount = 0;
    this->probePorts();
#ifdef VERBOSE_AHCI
    logDebugn("%! AHCI Driver instance initalized.", "[AHCI Driver]");
#endif
}

bool k_ahci_driver::initialize(uint8_t drive)
{
    if (!(drive < this->portCount))
        return false;

    k_ahci_port *port = this->ports[drive];

    if (port->initialized)
        return true;

    port->rebase();

    port->identity = (k_SATA_ident *)virtualAddressRangeAllocator.allocateRange(1, "ahci");
    pagingMapPage((virtual_address_t)port->identity, memoryPhysicalAllocator.allocatePage());
    memset((char *)port->identity, 0, 0x1000);

    port->identify();

#ifdef VERBOSE_AHCI
    logDebugn("%! Initialized drive on port %d with:\
                \n\t- Sector count: %d",
              "[AHCI Driver]", port->portNumber, port->identity->lba_capacity);

#endif

    return port->initialized;
}

bool k_ahci_driver::status(uint8_t drive)
{
    if (!(drive < this->portCount))
        return false;

    k_ahci_port *port = this->ports[drive];

    return port->initialized;
}

bool k_ahci_driver::read(uint8_t drive, uint64_t sector, uint32_t count, uint8_t *buf)
{
#ifdef VERBOSE_AHCI
    logDebugn("%! Trying to read from port %d", "[AHCI Driver]", drive);
    logDebugn("%! Port count is %d", "[AHCI Driver]", this->portCount);
#endif
    if (!(drive < this->portCount))
        return false;

    k_ahci_port *port = this->ports[drive];

    if (!port->initialized)
        return false;

    port->buffer = buf;
    uint32_t sectorl = (uint32_t)sector;
    uint32_t sectorh = (uint32_t)(sector >> 32);

    return port->read(sectorl, sectorh, count, buf);
}

bool k_ahci_driver::write(uint8_t drive, uint64_t sector, uint32_t count, uint8_t *buf)
{
    if (!(drive < this->portCount))
        return false;

    k_ahci_port *port = this->ports[drive];

    if (!port->initialized)
        return false;

    port->buffer = buf;
    uint32_t sectorl = (uint32_t)sector;
    uint32_t sectorh = (uint32_t)(sector >> 32);

    return port->write(sectorl, sectorh, count, buf);
}

// Check device type
static int checkType(k_HBA_port *port)
{
    uint32_t ssts = port->ssts;

    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;

    if (det != HBA_PORT_DET_PRESENT) // Check drive status
        return AHCI_DEV_NULL;
    if (ipm != HBA_PORT_IPM_ACTIVE)
        return AHCI_DEV_NULL;

    switch (port->sig)
    {
    case SATA_SIG_ATAPI:
        return AHCI_DEV_SATAPI;
    case SATA_SIG_SEMB:
        return AHCI_DEV_SEMB;
    case SATA_SIG_PM:
        return AHCI_DEV_PM;
    default:
        return AHCI_DEV_SATA;
    }
}

// Start command engine
void k_ahci_port::startCMD()
{
    // Wait until CR (bit15) is cleared
    while (this->hbaPort->cmd & HBA_PxCMD_CR)
        ;

    // Set FRE (bit4) and ST (bit0)
    this->hbaPort->cmd |= HBA_PxCMD_FRE;
    this->hbaPort->cmd |= HBA_PxCMD_ST;
}

// Stop command engine
void k_ahci_port::stopCMD()
{
    // Clear ST (bit0)
    this->hbaPort->cmd &= ~HBA_PxCMD_ST;

    // Clear FRE (bit4)
    this->hbaPort->cmd &= ~HBA_PxCMD_FRE;

    // Wait until FR (bit14), CR (starth
}

bool k_ahci_port::identify()
{
    this->hbaPort->is = 0xffff; // Clear pending interrupt bits

    // Find a free command slot
    int slot = this->findCMDslot();
    if (slot == -1)
        return false;

    // Edit the header for the command for the read
    k_HBA_cmd_header *cmdHeader = (k_HBA_cmd_header *)this->virtualCLB;
    cmdHeader += slot;
    cmdHeader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t); // Command FIS size
    cmdHeader->w = 0;                                        // Read from device
    cmdHeader->c = 1;                                        // Read from device
    cmdHeader->p = 1;                                        // Read from device
    cmdHeader->prdtl = 1;                                    // PRDT entries count

    // Edit the command table for the read
    k_HBA_cmd_table *cmdTable = this->virtualCTBs[slot];

    physical_address_t bufferPhysical = pagingVirtualToPhysical((virtual_address_t)this->identity);

    cmdTable->prdt_entry[0].dba = (uint32_t)(bufferPhysical & 0xFFFFFFFF);
    cmdTable->prdt_entry[0].dbau = (uint32_t)((bufferPhysical >> 32) & 0xFFFFFFFF);

    cmdTable->prdt_entry[0].dbc = 0x1FF; // multiply by 512 bytes per sector
    cmdTable->prdt_entry[0].i = 0;

    // Create the FIS (Frame Information Structure)
    FIS_REG_H2D *cmdFIS = (FIS_REG_H2D *)(&cmdTable->cfis);
    memset((char *)cmdFIS, 0, sizeof(FIS_REG_H2D));
    cmdFIS->fis_type = FIS_TYPE_REG_H2D;
    cmdFIS->c = 1; // Command
    cmdFIS->command = ATA_CMD_IDENTIFY_DEV;

    // Wait until the port is free
    uint64_t spin = 0; // Spinlock
    while ((hbaPort->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    if (spin == 1000000)
    {
        logInfon("%! Port %d is hung.", "[AHCI Driver]", this->portNumber);
        return false;
    }

    // Issue command and check for success
    this->hbaPort->ci = 1 << slot;
    while (1)
    {
        if ((this->hbaPort->ci & (1 << slot)) == 0)
            break;
    }
    return true;
}

void k_ahci_port::rebase()
{
    stopCMD(); // Stop command engine

    this->initialized = true;

    // Rebase the CLB (Command List Base Address)
    // Note that that address must be 1024byte-aligned and we complying with this
    // request since a page is 4096byte-aligned
    physical_address_t clbPhys = memoryPhysicalAllocator.allocatePage();
    if (!clbPhys)
    {
        this->initialized = false;
        return;
    }

    this->hbaPort->clb = clbPhys;
    this->hbaPort->clbu = clbPhys >> 32;

    virtual_address_t clbVirt = virtualAddressRangeAllocator.allocateRange(1, "ahci");
    if (!clbVirt)
    {
        this->initialized = false;
        return;
    }
    pagingMapPage(clbVirt, clbPhys);
    // Reset the memory
    memset((char *)clbVirt, 0, 1024);

    // Save a copy of the CLB virtual address
    this->virtualCLB = (k_HBA_cmd_header *)clbVirt;

    // Rebase the FB
    // Note that that address must be 256byte-aligned and we complying with this
    // request since a we are 1024byte-aligned
    physical_address_t fbPhys = clbPhys + 1024;
    this->hbaPort->fb = fbPhys;
    this->hbaPort->fbu = fbPhys >> 32;

    virtual_address_t fbVirt = clbVirt + 1024;
    // We don't need (and can't) to map a page, it the same page of the CLB
    // Reset the memory
    memset((char *)fbVirt, 0, 256);

    // Save a copy of the virtual address
    this->virtualFB = fbVirt;

    // Rebase all the CTBs (Command Table Base Addresses)
    k_HBA_cmd_header *cmdheader = this->virtualCLB;
    // There are in total 8KiB for the CTBs, so we will map 2 pages for it
    int CHi = 0;
    for (int t = 0; t < 2; t++)
    {
        physical_address_t ctbPhys = memoryPhysicalAllocator.allocatePage();
        virtual_address_t ctbVirt = virtualAddressRangeAllocator.allocateRange(1, "ahci");
        if (!ctbPhys || !ctbVirt)
        {
            this->initialized = false;
            return;
        }
        pagingMapPage(ctbVirt, ctbPhys);

        for (int offset = 0; offset < PAGE_SIZE; offset += 256)
        {
            cmdheader[CHi].prdtl = 8; // 8 PRDT entries per CTB

            // Set the pointer to the physical address of the command table
            cmdheader[CHi].ctba = ctbPhys + offset;
            cmdheader[CHi].ctbau = (ctbPhys + offset) >> 32;

            // Save a copy of the virtual address
            this->virtualCTBs[CHi] = (k_HBA_cmd_table *)(ctbVirt + offset);

            CHi++;
        }

        // Reset the memory
        memset((char *)ctbVirt, 0, PAGE_SIZE);
    }

    startCMD(); // Start command engine
}

void k_ahci_driver::probePorts()
{
    uint32_t pi = this->ABAR->pi;
    int i = 0;
#ifdef VERBOSE_AHCI
    logDebugn("%! Probing ports:", "[AHCI Driver]");
#endif
    while (i < 32)
    {
        if (pi & 1)
        {
            int dt = checkType(&this->ABAR->ports[i]);
#ifdef VERBOSE_AHCI
            switch (dt)
            {
            case AHCI_DEV_SATA:
                logDebugn("\t- SATA drive found at port %d", i);
                break;
            case AHCI_DEV_SATAPI:
                logDebugn("\t- SATAPI drive found at port %d", i);
                break;
            case AHCI_DEV_SEMB:
                logDebugn("\t- SEMB drive found at port %d", i);
                break;
            case AHCI_DEV_PM:
                logDebugn("\t- PM drive found at port %d", i);
                break;
            default:
                logDebugn("\t- No device was found at port %d", i);
                break;
            }
#endif

            if (dt == AHCI_DEV_SATA) // || dt == AHCI_DEV_SATAPI)
            {
                // Create port and add it to our array
                this->ports[portCount] = new k_ahci_port();
                this->ports[portCount]->type = dt;
                this->ports[portCount]->portNumber = i;
                this->ports[portCount]->hbaPort = &this->ABAR->ports[i];
                this->ports[portCount]->cmdSlots = this->cmdSlots;
                this->ports[portCount]->initialized = false;
                this->ports[portCount]->inWrite = false;
                this->portCount++;
            }
        }
        pi >>= 1;
        i++;
    }
}

int k_ahci_port::findCMDslot()
{
    // If not set in SACT and CI, the slot is free
    uint32_t slots = (this->hbaPort->sact | this->hbaPort->ci);
    for (int i = 0; i < this->cmdSlots; i++)
    {
        if ((slots & 1) == 0)
            return i;
        slots >>= 1;
    }
    logWarnn("%! Cannot find free command list entry on port %d.", "[AHCI]", this->portNumber);
    return -1;
}

bool k_ahci_port::read(uint32_t startl, uint32_t starth, uint32_t count, uint8_t *buf)
{
#ifdef VERBOSE_AHCI
    logDebugn("Reading started in port %d", this->portNumber);
#endif
    this->hbaPort->is = 0xffff; // Clear pending interrupt bits

#ifdef VERBOSE_AHCI
    logDebugn("Looking for an empty command slot");
#endif
    // Find a free command slot
    int slot = this->findCMDslot();
    if (slot == -1)
        return false;
#ifdef VERBOSE_AHCI
    logDebugn("Found command slot %d", slot);
#endif

    // Edit the header for the command for the read
    k_HBA_cmd_header *cmdHeader = (k_HBA_cmd_header *)this->virtualCLB;
    cmdHeader += slot;
    cmdHeader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t); // Command FIS size
    cmdHeader->w = 0;                                        // Read from device
    cmdHeader->c = 1;                                        // Read from device
    cmdHeader->p = 1;                                        // Read from device
    cmdHeader->prdtl = (uint16_t)((count - 1) >> 4) + 1;     // PRDT entries count

    // Edit the command table for the read
    k_HBA_cmd_table *cmdTable = this->virtualCTBs[slot];

    physical_address_t bufferPhysical = pagingVirtualToPhysical((virtual_address_t)buf);
    int i = 0;
    for (i = 0; i < cmdHeader->prdtl - 1; i++)
    {
        cmdTable->prdt_entry[i].dba = (uint32_t)(bufferPhysical & 0xFFFFFFFF);
        cmdTable->prdt_entry[i].dbau = (uint32_t)((bufferPhysical >> 32) & 0xFFFFFFFF);
        cmdTable->prdt_entry[i].dbc = 8 * 1024 - 1; // 8K bytes
        cmdTable->prdt_entry[i].i = 0;
        bufferPhysical += 4 * 1024; // 4K words
        count -= 16;                // 16 sectors
    }
    cmdTable->prdt_entry[i].dba = (uint32_t)(bufferPhysical & 0xFFFFFFFF);
    cmdTable->prdt_entry[i].dbau = (uint32_t)((bufferPhysical >> 32) & 0xFFFFFFFF);

    cmdTable->prdt_entry[i].dbc = count << 9; // multiply by 512 bytes per sector
    cmdTable->prdt_entry[i].i = 0;

    // Create the FIS (Frame Information Structure)
    FIS_REG_H2D *cmdFIS = (FIS_REG_H2D *)(&cmdTable->cfis);
    cmdFIS->fis_type = FIS_TYPE_REG_H2D;
    cmdFIS->c = 1; // Command
    cmdFIS->command = ATA_CMD_READ_DMA_EX;

    cmdFIS->lba0 = (uint8_t)startl;
    cmdFIS->lba1 = (uint8_t)(startl >> 8);
    cmdFIS->lba2 = (uint8_t)(startl >> 16);
    cmdFIS->device = 1 << 6; // LBA mode
    cmdFIS->lba3 = (uint8_t)(startl >> 24);
    cmdFIS->lba4 = (uint8_t)starth;
    cmdFIS->lba5 = (uint8_t)(starth >> 8);

    cmdFIS->countl = count & 0xff;
    cmdFIS->counth = count >> 8;

    // Wait until the port is free
    uint64_t spin = 0; // Spinlock
    while ((hbaPort->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    if (spin == 1000000)
    {
        logInfon("%! Port %d is hung.", "[AHCI Driver]", this->portNumber);
        return false;
    }

    // Issue command and check for success
    this->hbaPort->ci = 1 << slot;
    while (1)
    {
        if ((this->hbaPort->ci & (1 << slot)) == 0)
            break;
        if (this->hbaPort->is & HBA_PxIS_TFES)
        { // Task file error
            logInfon("%! Read disk error.", "[AHCI Driver]");
            return false;
        }
    }
    if (this->hbaPort->is & HBA_PxIS_TFES)
    {
        logInfon("%! Read disk error.", "[AHCI Driver]");
        return false;
    }
#ifdef VERBOSE_AHCI
    logDebugn("Reading ended successfully in port %d", this->portNumber);
#endif
    return true;
}

uint64_t k_ahci_port::getSectorCount()
{
    if (!this->initialized)
        return 0;
    return (uint64_t)(this->identity->lba_capacity);
}

uint64_t k_ahci_driver::getSectorCount(uint8_t drive)
{
    if (!(drive < this->portCount))
        return false;

    k_ahci_port *port = this->ports[drive];

    if (!port->initialized)
        return false;

    return port->getSectorCount();
}

bool k_ahci_port::write(uint32_t startl, uint32_t starth, uint32_t count, uint8_t *buf)
{
#ifdef VERBOSE_AHCI
    logDebugn("Writing started in port %d", this->portNumber);
#endif
    this->inWrite = true;
    this->hbaPort->is = 0xffff; // Clear pending interrupt bits
#ifdef VERBOSE_AHCI
    logDebugn("Looking for command slot");
#endif
    // Find a free command slot
    int slot = this->findCMDslot();
    if (slot == -1)
        return false;
#ifdef VERBOSE_AHCI
    logDebugn("Found command slot %d", slot);
#endif

    // Edit the header for the command for the read
    k_HBA_cmd_header *cmdHeader = (k_HBA_cmd_header *)this->virtualCLB;
    cmdHeader += slot;
    cmdHeader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t); // Command FIS size
    cmdHeader->w = 1;                                        // Write to device
    cmdHeader->c = 1;                                        // Write to device
    cmdHeader->p = 1;                                        // Write to device
    cmdHeader->prdtl = (uint16_t)((count - 1) >> 4) + 1;     // PRDT entries count

    // Edit the command table for the read
    k_HBA_cmd_table *cmdTable = this->virtualCTBs[slot];

    physical_address_t bufferPhysical = pagingVirtualToPhysical((virtual_address_t)buf);
    int i = 0;
    for (i = 0; i < cmdHeader->prdtl - 1; i++)
    {
        cmdTable->prdt_entry[i].dba = (uint32_t)(bufferPhysical & 0xFFFFFFFF);
        cmdTable->prdt_entry[i].dbau = (uint32_t)((bufferPhysical >> 32) & 0xFFFFFFFF);
        cmdTable->prdt_entry[i].dbc = 8 * 1024 - 1; // 8K bytes
        cmdTable->prdt_entry[i].i = 0;
        bufferPhysical += 4 * 1024; // 4K words
        count -= 16;                // 16 sectors
    }
    cmdTable->prdt_entry[i].dba = (uint32_t)(bufferPhysical & 0xFFFFFFFF);
    cmdTable->prdt_entry[i].dbau = (uint32_t)((bufferPhysical >> 32) & 0xFFFFFFFF);

    cmdTable->prdt_entry[i].dbc = count << 9; // multiply by 512 bytes per sector
    cmdTable->prdt_entry[i].i = 0;

    // Create the FIS (Frame Information Structure)
    FIS_REG_H2D *cmdFIS = (FIS_REG_H2D *)(&cmdTable->cfis);
    cmdFIS->fis_type = FIS_TYPE_REG_H2D;
    cmdFIS->c = 1; // Command
    cmdFIS->command = ATA_CMD_WRITE_DMA_EX;

    cmdFIS->lba0 = (uint8_t)startl;
    cmdFIS->lba1 = (uint8_t)(startl >> 8);
    cmdFIS->lba2 = (uint8_t)(startl >> 16);
    cmdFIS->device = 1 << 6; // LBA mode
    cmdFIS->lba3 = (uint8_t)(startl >> 24);
    cmdFIS->lba4 = (uint8_t)starth;
    cmdFIS->lba5 = (uint8_t)(starth >> 8);

    cmdFIS->countl = count & 0xff;
    cmdFIS->counth = count >> 8;

    // Wait until the port is free
    uint64_t spin = 0; // Spinlock
    while ((hbaPort->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    if (spin == 1000000)
    {
        logWarnn("%! Port %d is hung.", "[AHCI Driver]", this->portNumber);
        return false;
    }

    // Issue command and check for success
    this->hbaPort->ci = 1 << slot;
    while (1)
    {
        if ((this->hbaPort->ci & (1 << slot)) == 0)
            break;
        if (this->hbaPort->is & HBA_PxIS_TFES)
        { // Task file error
            logWarnn("%! Write disk error, \n\
            SERRR: %d, IS: %d, CI: %d.",
                     "[AHCI Driver]", hbaPort->serr, hbaPort->is, hbaPort->ci);
            return false;
        }
    }
    if (this->hbaPort->is & HBA_PxIS_TFES)
    {
        logInfon("%! Write disk error, TFD: %d.", "[AHCI Driver]", hbaPort->tfd);
        return false;
    }

#ifdef VERBOSE_AHCI
    logDebugn("Writing ended successfully in port %d", this->portNumber);
#endif
    this->inWrite = false;
    return true;
}