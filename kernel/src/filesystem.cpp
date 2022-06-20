#include <filesystem.hpp>

#include <fatfs/ff.h>
#include <logger/logger.hpp>

#include <stdint.h>

#include <kernel.hpp>
#include <strings.hpp>
#include <logger/printf.hpp>
#include <storage/ahci/ahci.hpp>

static FATFS fs;

FRESULT scan_files(
    char *path /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path); /* Open the directory */
    if (res == FR_OK)
    {
        for (;;)
        {
            res = f_readdir(&dir, &fno); /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0)
            {
                logInfon("%s/", path);
                break; /* Break on error or end of dir */
            }
            if (fno.fattrib & AM_DIR)
            { /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path); /* Enter the directory */
                if (res != FR_OK)
                    break;
                path[i] = 0;
            }
            else
            { /* It is a file. */
                logInfon("%s/%s", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}

void filesystemInitialize()
{

    PCICommonConfig *ahciDevice = pciGetDevice(0x01, 0x06, 0x01);
    if (ahciDevice == NULL)
        kernelPanic("AHCI is wrong (address null)");
    mainDriver = new k_ahci_driver(ahciDevice);

    logDebugn("Finished creating the mainDriver");

    FRESULT res;
    uint64_t bw;
    uint8_t work[FF_MAX_SS];

    res = f_mount(&fs, "", 1);
    if (res == FR_NO_FILESYSTEM)
    {
        // Create filesystem
        res = f_mkfs("", 0, work, sizeof(work));
        if (res == FR_OK)
            logDebugn("%! Created filesystem", "[Filesystem]", res);
        else
            kernelPanic("%! Error while creating filesystem code: %d.", "[Filesystem]", res);
    }
    else
    {
        logDebugn("%! Filesystem was already created", "[Filesystem]", res);
    }

    FILINFO fn;
    f_mkdir("root");
    f_mkdir("root/apps");
    f_mkdir("root/docs");
    f_mkdir("root/dev");
    
    // scan_files("/root");
}