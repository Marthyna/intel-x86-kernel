.PHONY: clean all kill run docker_build remote_run


# Output directory for each submakefiles
OUTPUT := out
export OUTPUT

QEMU = qemu-system-i386
KERNEL = kernel/kernel.bin

QEMUOPTSRUN = -machine q35 -m 256 -kernel $(KERNEL)
QEMUOPTS = $(QEMUOPTSRUN) -curses
QEMUOPTSDEBUG = $(QEMUOPTSRUN) -s -S
DIR = $(shell git rev-parse --show-toplevel)

DOCKER_IMG_NAME=ubuntu
DOCKER_CTN_NAME=x86_64-dev-env

#
# Some build tools need to be explicitely defined before building. The toolchain
# creates the following platform tools configuration file before it allows the
# toolchain to build.
#
PLATFORM_TOOLS := $(OUTPUT)/platform-tools.mk
export PLATFORM_TOOLS

run: 
	$(QEMU) $(QEMUOPTSRUN)

debug:
	$(QEMU) $(QEMUOPTSDEBUG) &
	emacs --eval '(progn (gdb "gdb -i=mi $(KERNEL)") (insert "target remote :1234") (gdb-many-windows))'

remote_run:
	$(QEMU) $(QEMUOPTS)

docker_build:
	docker build -t $(DOCKER_IMG_NAME) . --platform linux/amd64

docker_all: | kernel/$(PLATFORM_TOOLS) user/$(PLATFORM_TOOLS)
	@docker container run \
		--rm -it \
		--platform linux/amd64 \
		--cpus="4" \
		-v $(DIR):/home/ \
		ubuntu \
		make all

mac_deb:
	$(QEMU) $(QEMUOPTSDEBUG) &
	i386-elf-gdb $(KERNEL) \
		-ex 'target remote :1234' \
		-ex 'directory kernel' \
		-ex 'break kernel_start' \
		-ex 'continue' \
		-ex 'layout src'
	make kill

all: | kernel/$(PLATFORM_TOOLS) user/$(PLATFORM_TOOLS)
	$(MAKE) -C user/ all VERBOSE=$(VERBOSE)
	$(MAKE) -C kernel/ kernel.bin VERBOSE=$(VERBOSE)

kernel/$(PLATFORM_TOOLS):
	$(MAKE) -C kernel/ $(PLATFORM_TOOLS)

user/$(PLATFORM_TOOLS):
	$(MAKE) -C user/ $(PLATFORM_TOOLS)

kill:
	killall qemu-system-i386

clean:
	$(MAKE) clean -C kernel/
	$(MAKE) clean -C user/

