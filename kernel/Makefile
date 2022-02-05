# This is the name that our final kernel executable will have.
# Change as needed.
override KERNEL := kernel.elf

# It is highly recommended to use a custom built cross toolchain to build a kernel.
# We are only using "cc" as a placeholder here. It may work by using
# the host system's toolchain, but this is not guaranteed.
ifeq ($(origin CC), default)
CC := g++
endif

# Likewise, "ld" here is just a placeholder and your mileage may vary if using the
# host's "ld".
ifeq ($(origin LD), default)
LD := ld
endif

vpath %.cpp $(sort $(dir $(shell find ./src -type f -name '*')))
vpath %.h 	include
vpath %.hpp include
vpath %.o 	lib
vpath %.d 	lib
vpath %.elf bin

.SUFFIXES:
.SUFFIXES: .cpp .h .hpp .o .d .elf

SRCDIR          = src
INCLUDESDIR     = include
BUILDDIR        = bin
OBJDIR          = lib

# User controllable CFLAGS.
CFLAGS ?= -Wall -Wextra -O0 -pipe -g

# User controllable linker flags. We set none by default.
LDFLAGS ?=

# Internal C flags that should not be changed by the user.
override INTERNALCFLAGS :=   \
	-I $(INCLUDESDIR)    \
	-std=c++98           \
	-ffreestanding       \
	-fno-stack-protector \
	-fno-pic             \
	-mabi=sysv           \
	-mno-80387           \
	-mno-mmx             \
	-mno-3dnow           \
	-mno-sse			 \
	-mno-sse2			 \
	-mno-red-zone        \
	-mcmodel=kernel      \
	-MMD



# Internal linker flags that should not be changed by the user.
override INTERNALLDFLAGS :=    \
	-I $(INCLUDESDIR)      \
	-T kernel.ld           \
	-nostdlib              \
	-zmax-page-size=0x1000 \
	-static

# Use find to glob all *.c files in the directory and extract the object names.
SRCS 			= $(shell find ./ -type f -name '*.cpp')
OBJS 			= $(patsubst %.cpp, %.o, $(notdir $(SRCS)))
HEADER_DEPS 	= $(OBJS:.cpp=.d)

# Default target.
.PHONY: all
all: $(KERNEL)

stivale2.h:
	curl https://raw.githubusercontent.com/stivale/stivale/master/stivale2.h -o $(INCLUDESDIR)/$@

# Link rules for the final kernel executable.
$(KERNEL): $(OBJS)
	$(LD) $(addprefix $(OBJDIR)/, $(notdir $^)) $(LDFLAGS) $(INTERNALLDFLAGS) -o $(BUILDDIR)/$@

%.o: %.cpp stivale2.h
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $< -o $(OBJDIR)/$@
# Compilation rules for *.c files.
#include $(HEADER_DEPS)


# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf $(BUILDDIR)/$(KERNEL) $(OBJDIR)/*
.PHONY: distclean
distclean: clean
	rm -f stivale2.h

test: $(OBJS)
	echo $(OBJDIR)