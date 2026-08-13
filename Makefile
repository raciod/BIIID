# Directories
KERNEL_DIR := kernel
CLI_DIR    := cli
SHARED_DIR := shared
TESTS_DIR  := tests

# Toolchain
CC     := gcc
CFLAGS := -Wall -Wextra -O2 -I$(SHARED_DIR)

# Binaries
CLI_BIN := $(CLI_DIR)/fwctl

.PHONY: all kernel cli clean install remove log test

# Default target builds both kernel module and user-space CLI
all: kernel cli

# Build kernel module
kernel:
	$(MAKE) -C $(KERNEL_DIR)

# Build CLI tool
cli: $(CLI_BIN)

$(CLI_BIN): $(CLI_DIR)/fwctl.c
	$(CC) $(CFLAGS) $< -o $@

# Kernel management helper rules
install:
	sudo insmod $(KERNEL_DIR)/netfilter_firewall.ko

remove:
	sudo rmmod netfilter_firewall || true

log:
	sudo dmesg | tail -n 20

test:
	sudo $(TESTS_DIR)/test_firewall.sh

# Cleanup all generated files
clean:
	$(MAKE) -C $(KERNEL_DIR) clean
	rm -f $(CLI_BIN)
