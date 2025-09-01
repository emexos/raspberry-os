SRCDIR = src
BOOTDIR = boot
BUILDDIR = build
KERNELDIR = $(SRCDIR)/kernel
LIBDIR = $(SRCDIR)/lib
INCDIR = $(SRCDIR)/include
DRIVERDIR = $(SRCDIR)/drivers
INPUTDIR = $(SRCDIR)/input
GUIDIR = $(SRCDIR)/gui

BUILDBOTDIR = $(BUILDDIR)/boot
BUILDKERNELDIR = $(BUILDDIR)/kernel
BUILDLIBDIR = $(BUILDDIR)/lib
BUILDDRIVERDIR = $(BUILDDIR)/drivers
BUILDINPUTDIR = $(BUILDDIR)/input
BUILDGUIDIR = $(BUILDDIR)/gui

# Disable implicit rules to prevent conflicts
MAKEFLAGS += --no-builtin-rules
.SUFFIXES:

# Find all C files recursively
CFILES = $(shell find $(KERNELDIR) $(LIBDIR) $(DRIVERDIR) $(INPUTDIR) $(GUIDIR) -name "*.c" 2>/dev/null || true)

# Generate object file paths
KERNEL_OFILES = $(patsubst $(KERNELDIR)/%.c,$(BUILDKERNELDIR)/%.o,$(wildcard $(KERNELDIR)/*.c))
LIB_OFILES = $(patsubst $(LIBDIR)/%.c,$(BUILDLIBDIR)/%.o,$(wildcard $(LIBDIR)/*.c))
DRIVER_OFILES = $(patsubst $(DRIVERDIR)/%.c,$(BUILDDRIVERDIR)/%.o,$(wildcard $(DRIVERDIR)/**/*.c))
INPUT_USB_OFILES = $(patsubst $(INPUTDIR)/usb/%.c,$(BUILDINPUTDIR)/usb/%.o,$(wildcard $(INPUTDIR)/usb/*.c))
GUI_DESKTOP_OFILES = $(patsubst $(GUIDIR)/desktop/%.c,$(BUILDGUIDIR)/desktop/%.o,$(wildcard $(GUIDIR)/desktop/*.c))
GUI_LOGIN_OFILES = $(patsubst $(GUIDIR)/login/%.c,$(BUILDGUIDIR)/login/%.o,$(wildcard $(GUIDIR)/login/*.c))
GUI_MAIN_OFILES = $(patsubst $(GUIDIR)/%.c,$(BUILDGUIDIR)/%.o,$(wildcard $(GUIDIR)/*.c))

OFILES = $(KERNEL_OFILES) $(LIB_OFILES) $(DRIVER_OFILES) $(INPUT_USB_OFILES) $(GUI_DESKTOP_OFILES) $(GUI_LOGIN_OFILES) $(GUI_MAIN_OFILES)

LLVMPATH = /opt/homebrew/opt/llvm/bin
LLDPATH = /opt/homebrew/opt/lld/bin
CLANGFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a72+nosimd -I$(INCDIR)

QEMU = qemu-system-aarch64
QEMU_FLAGS = -M raspi4b -cpu cortex-a72 -m 2G -serial stdio -kernel kernel8.img

all: clean kernel8.img run

$(BUILDDIR):
	@mkdir -p $@

$(BUILDBOTDIR)/boot.o: $(BOOTDIR)/boot.S | $(BUILDDIR)
	@mkdir -p $(BUILDBOTDIR)
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c $< -o $@

$(BUILDKERNELDIR)/%.o: $(KERNELDIR)/%.c | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c $< -o $@

$(BUILDLIBDIR)/%.o: $(LIBDIR)/%.c | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c $< -o $@

$(BUILDDRIVERDIR)/%.o: $(DRIVERDIR)/%.c | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c $< -o $@

$(BUILDINPUTDIR)/usb/%.o: $(INPUTDIR)/usb/%.c | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c $< -o $@

$(BUILDGUIDIR)/desktop/%.o: $(GUIDIR)/desktop/%.c | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c $< -o $@

$(BUILDGUIDIR)/login/%.o: $(GUIDIR)/login/%.c | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c $< -o $@

$(BUILDGUIDIR)/%.o: $(GUIDIR)/%.c | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c $< -o $@

kernel8.img: $(BUILDBOTDIR)/boot.o $(OFILES)
	$(LLDPATH)/ld.lld -m aarch64elf -nostdlib $(BUILDBOTDIR)/boot.o $(OFILES) -T $(BOOTDIR)/link.ld -o kernel8.elf
	$(LLVMPATH)/llvm-objcopy -O binary kernel8.elf kernel8.img

run: kernel8.img
	$(QEMU) $(QEMU_FLAGS) -display cocoa,zoom-to-fit=on

clean:
	@rm -f kernel8.elf kernel8.img
	@rm -rf $(BUILDDIR)

debug-files:
	@echo "C files: $(CFILES)"
	@echo "Object files: $(OFILES)"

create-structure:
	@mkdir -p $(BOOTDIR) $(SRCDIR)/drivers $(INCDIR) $(KERNELDIR) $(LIBDIR) $(BUILDDIR)
	@mkdir -p $(INPUTDIR)/usb $(GUIDIR)/desktop $(GUIDIR)/login

test-usb: kernel8.img
	$(QEMU) $(QEMU_FLAGS) -display cocoa,zoom-to-fit=on -device qemu-xhci -device usb-kbd -device usb-mouse

.PHONY: all clean run debug-files create-structure test-usb
