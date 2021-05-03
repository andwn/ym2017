MARSDEV ?= ${HOME}/mars
MARSBIN  = $(MARSDEV)/m68k-elf/bin
TOOLSBIN = $(MARSDEV)/bin

CC   = $(MARSBIN)/m68k-elf-gcc
AS   = $(MARSBIN)/m68k-elf-as
LD   = $(MARSBIN)/m68k-elf-ld
NM   = $(MARSBIN)/m68k-elf-nm
OBJC = $(MARSBIN)/m68k-elf-objcopy

ASMZ80  = $(TOOLSBIN)/sjasm
BINTOS  = $(TOOLSBIN)/bintos
RESCOMP = java -jar $(TOOLSBIN)/rescomp.jar
XGMTOOL = $(TOOLSBIN)/xgmtool
WAVTORAW= $(TOOLSBIN)/wavtoraw

GENGCC_VER := $(shell $(CC) -dumpversion)
PLUGIN=$(MARSDEV)/m68k-elf/libexec/gcc/m68k-elf/$(GENGCC_VER)

INCS = -Isrc -Ires -Iinc
CCFLAGS = -m68000 -Wall -Wextra -O3 -std=c99 -c -fno-builtin -fomit-frame-pointer \
			-fno-web -fno-gcse -fno-unit-at-a-time -flto -fuse-linker-plugin -fshort-enums
ASFLAGS = -m68000 --register-prefix-optional
LIBS = -L$(MARSDEV)/m68k-elf/lib/gcc/m68k-elf/$(GENGCC_VER) -lgcc 
LINKFLAGS = -T $(MARSDEV)/ldscripts/sgdk.ld -nostdlib
FLAGSZ80 = -isrc/xgm

BOOTSS=$(wildcard src/boot/*.s)
BOOT_RESOURCES=$(BOOTSS:.s=.o)

RESS=$(wildcard res/*.res)
Z80S=$(wildcard src/xgm/*.s80)
CS=$(wildcard src/*.c)
CS+=$(wildcard src/xgm/*.c)
SS=$(wildcard src/*.s)
SS+=$(wildcard src/xgm/*.s)
RESOURCES=$(RESS:.res=.o)
RESOURCES+=$(Z80S:.s80=.o)
RESOURCES+=$(CS:.c=.o)
RESOURCES+=$(SS:.s=.o)

OBJS = $(RESOURCES)

.PHONY: all clean

all: ym2017.bin symbol.txt

# Cross reference symbol.txt with the addresses displayed in the crash handler
symbol.txt:
	$(NM) --plugin=$(PLUGIN)/liblto_plugin.so -n ym2017.elf > symbol.txt

src/boot/sega.o: src/boot/rom_head.bin
	$(AS) $(ASFLAGS) src/boot/sega.s -o $@

%.bin: %.elf
	$(OBJC) -S -O binary $< temp.bin
	dd if=temp.bin of=$@ bs=8192 conv=sync

%.elf: $(BOOT_RESOURCES) $(OBJS) 
	$(CC) -o $@ $(LINKFLAGS) $(BOOT_RESOURCES) $(OBJS) $(LIBS)

%.o: %.c
	@echo "CC $<"
	@$(CC) $(CCFLAGS) $(INCS) -c $< -o $@

%.o: %.s 
	@echo "AS $<"
	@$(AS) $(ASFLAGS) $< -o $@

%.s: %.res
	$(RESCOMP) $< $@

%.o80: %.s80
	$(ASMZ80) $(FLAGSZ80) $< $@ out.lst

%.s: %.o80
	$(BINTOS) $<

src/boot/rom_head.o: src/boot/rom_head.c
	$(CC) $(INCS) -m68000 -Wall -Wextra -std=c99 -c -fno-builtin -o $@ $<

src/boot/rom_head.bin: src/boot/rom_head.o
	$(LD) $(LINKFLAGS) --oformat binary -o $@ $<


clean:
	rm -f $(RESOURCES)
	rm -f ym2017.bin ym2017.elf temp.bin symbol.txt
	rm -f src/boot/sega.o src/boot/rom_head.o src/boot/rom_head.bin
	rm -f src/xgm/z80_drv.s src/xgm/z80_drv.o80 src/xgm/z80_drv.h out.lst
	rm -f res/resources.h res/resources.s
