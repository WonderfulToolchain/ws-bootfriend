ifndef WONDERFUL_TOOLCHAIN
$(error Please define WONDERFUL_TOOLCHAIN to point to the location of the Wonderful toolchain.)
endif

include $(WONDERFUL_TOOLCHAIN)/i8086/wswan.mk

OBJDIR := obj_wswan
MKDIRS := $(OBJDIR)
LIBS := -lws -lc -lgcc
CFLAGS += -Os -fno-jump-tables -ffunction-sections
LDFLAGS += -Wl,--gc-sections -Wl,--print-gc-sections
SLFLAGS := --heap-length 0x1800 --color --rom-size 2 --ram-type SRAM_8KB --unlock-ieep
SLFLAGS += --disable-custom-bootsplash
TARGET := bootfriend_inst.wsc

SRCDIRS := res src
CSOURCES := $(foreach dir,$(SRCDIRS),$(notdir $(wildcard $(dir)/*.c)))
ASMSOURCES := $(foreach dir,$(SRCDIRS),$(notdir $(wildcard $(dir)/*.S)))
OBJECTS := $(CSOURCES:%.c=$(OBJDIR)/%.o) $(ASMSOURCES:%.S=$(OBJDIR)/%.o)

CFLAGS += -Ires -Isrc

DEPS := $(OBJECTS:.o=.d)
CFLAGS += -MMD -MP

vpath %.c $(SRCDIRS)
vpath %.S $(SRCDIRS)

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(SWANLINK) -v -o $@ --output-elf $@.elf $(SLFLAGS) --linker-args $(LDFLAGS) $(WF_CRT0) $(OBJECTS) $(LIBS)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.S | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	$(info $(shell mkdir -p $(MKDIRS)))

clean:
	rm -r $(OBJDIR)/*
	rm $(TARGET)

-include $(DEPS)
