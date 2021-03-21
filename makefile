# Makefile uMPS3


XT_PRG_PREFIX = mipsel-linux-gnu-
CC = $(XT_PRG_PREFIX)gcc
LD = $(XT_PRG_PREFIX)ld


ifneq ($(wildcard /usr/bin/umps3),)
    UMPS3_DIR_PREFIX = /usr
else
    UMPS3_DIR_PREFIX = /usr/local
endif

UMPS3_DATA_DIR = $(UMPS3_DIR_PREFIX)/share/umps3
UMPS3_INCLUDE_DIR = $(UMPS3_DIR_PREFIX)/include/umps3
UMPS3_INCLUDE_DIR2 = $(UMPS3_DIR_PREFIX)/include

SRC_DIR = src
PHASE1_DIR = $(SRC_DIR)/phase1
PHASE2_DIR = $(SRC_DIR)/phase2
TESTING_DIR = $(SRC_DIR)/testers
PROJ_INCLUDE = -I$(SRC_DIR) -I$(PHASE1_DIR) -I$(PHASE2_DIR)

CFLAGS_LANG = -ffreestanding
CFLAGS_MIPS = -mips1 -mabi=32 -mno-gpopt -G 0 -mno-abicalls -fno-pic -mfp32
CFLAGS = $(CFLAGS_LANG) $(CFLAGS_MIPS) -I$(UMPS3_INCLUDE_DIR) -I$(UMPS3_INCLUDE_DIR2) $(PROJ_INCLUDE)  -Wall -O0 -std=gnu11

LDFLAGS = -G 0 -nostdlib -T $(UMPS3_DATA_DIR)/umpscore.ldscript

VPATH = $(UMPS3_DATA_DIR):$(SRC_DIR):$(PHASE1_DIR):$(PHASE2_DIR):$(TESTING_DIR)
OUTPUT_DIR = out
UMPS3_OBJ = crtso.o libumps.o
.PHONY : all clean

all : kernel.core.umps

kernel.core.umps : kernel
	umps3-elf2umps -k out/$<

kernel : asl.o p1test.o pcb.o crtso.o libumps.o
	$(LD) -o $(addprefix $(OUTPUT_DIR)/, $@) $(addprefix $(OUTPUT_DIR)/obj/, $^) $(LDFLAGS)

clean :
	-rm -f *.o kernel kernel.*.umps out -r

%.o : %.c
	@ mkdir -p $(OUTPUT_DIR)
	@ mkdir -p $(OUTPUT_DIR)/obj
	$(CC) $(CFLAGS) -c -o $(addprefix $(OUTPUT_DIR)/obj/, $@) $<

%.o : %.S
	@ mkdir -p $(OUTPUT_DIR)
	@ mkdir -p $(OUTPUT_DIR)/obj
	$(CC) $(CFLAGS) -c -o $(addprefix $(OUTPUT_DIR)/obj/, $@) $<