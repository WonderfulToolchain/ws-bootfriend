BUILDDIR := build
NASM := nasm
PYTHON3 := python3

.PHONY: all clean

all: bootfriend.bin

bootfriend.bin: bootfriend.asm
	@mkdir -p $(BUILDDIR)
	$(NASM) -M -MG -o $@ bootfriend.asm > $(BUILDDIR)/main.d
	$(NASM) -o $@ bootfriend.asm

clean:
	rm -r $(BUILDDIR)

-include $(BUILDDIR)/main.d
