BUILDDIR = ./build
OSNAME = MyOS

# Kernel directory
KERNELDIR = kernel
KERNELDATADIR = $(KERNELDIR)/bin/data/*
KERNELDATA = $(wildcard ${KERNELDATADIR})


# Bootsector properties
LIMINEDIR = bootloader
LIMINEEFI = $(LIMINEDIR)/BOOTX64.EFI
LIMINECFG = $(LIMINEDIR)/limine.cfg
LIMINEDATADIR = $(LIMINEDIR)/data/*
LIMINEDATA = $(wildcard ${LIMINEDATADIR})
STARTUPNSH = ./startup.nsh

# Image properties
IMAGESIZE = 10000 # The size of the OS image, in KiB

# QEMU properties
OVMFDIR = ./OVMF

.PHONY: buildimg
buildimg: 
	if [!-d "build"]; then @mkdir build; fi
	@echo "Creating an empty image..."
# Create a file with size 512kB containin zeros
	@dd if=/dev/zero of=$(BUILDDIR)/$(OSNAME).img bs=1024 count=$(IMAGESIZE) 2>/dev/null
# Format the file as a DOS/MBR bootsector
	@echo "Formating the image..."
	@echo "* Creating the filesystem"
	@mformat -i $(BUILDDIR)/$(OSNAME).img ::
	@echo "* Creating /EFI directory"
	@mmd -i $(BUILDDIR)/$(OSNAME).img ::/EFI
	@echo "* Creating /EFI/BOOT directory"
	@mmd -i $(BUILDDIR)/$(OSNAME).img ::/EFI/BOOT
	@echo "* Creating /KERNEL directory"
	@mmd -i $(BUILDDIR)/$(OSNAME).img ::/KERNEL
	@echo "* Creating /KERNEL/DATA directory"
	@mmd -i $(BUILDDIR)/$(OSNAME).img ::/KERNEL/DATA
# Copy all the data into it
	@echo "Copying bootloader into image..."
	@echo "* Copying bootloader binary into"
	@mcopy -i $(BUILDDIR)/$(OSNAME).img $(LIMINEEFI) ::/EFI/BOOT
	@echo "* Copying startup.nsh"
	@mcopy -i $(BUILDDIR)/$(OSNAME).img $(STARTUPNSH) ::
	@echo "* Copying Limine config" 
	@mcopy -i $(BUILDDIR)/$(OSNAME).img $(LIMINECFG) ::
	@echo "* Copying $(LIMINEDATA) into /EFI/BOOT"; 
	@mcopy -i $(BUILDDIR)/$(OSNAME).img $(LIMINEDATA) ::/EFI/BOOT; 

	@echo "Copying kernel.elf into image..."
	@mcopy -i $(BUILDDIR)/$(OSNAME).img $(KERNELDIR)/bin/kernel.elf ::KERNEL 
	@echo "Copying kernel ${KERNELDATA} into image..."
	@mcopy -i $(BUILDDIR)/$(OSNAME).img $(KERNELDATA) ::/KERNEL/DATA

.PHONY: run
run:
	qemu-system-x86_64 -drive file=$(BUILDDIR)/$(OSNAME).img -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="$(OVMFDIR)/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="$(OVMFDIR)/OVMF_VARS-pure-efi.fd" -net none

.PHONY: qemudebug
qemudebug:
	@echo Launched QEMU with GDB server on :1234
	@qemu-system-x86_64 -drive file=$(BUILDDIR)/$(OSNAME).img \
	-s -S -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="$(OVMFDIR)/OVMF_CODE-pure-efi.fd",readonly=on \
	-drive if=pflash,format=raw,unit=1,file="$(OVMFDIR)/OVMF_VARS-pure-efi.fd" -net none
