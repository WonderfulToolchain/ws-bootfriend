BUILDDIR := build
NASM := nasm
PYTHON3 := python3

.PHONY: all clean

all: bootfriend_template.bin bootfriend.bin

bootfriend_template.bin: bootfriend.asm bootfriend.bin
	$(NASM) -o $@ bootfriend.asm

bootfriend.bin: bootfriend.asm
	@mkdir -p $(BUILDDIR)
	$(NASM) -M -MG -o $@ bootfriend.asm > $(BUILDDIR)/main.d
	$(NASM) -DROM -o $@ bootfriend.asm

clean:
	rm -r $(BUILDDIR)

-include $(BUILDDIR)/main.d
