# Directories
KERNEL_DIR := kernel
CLI_DIR    := fwctl
SHARED_DIR := shared
TESTS_DIR  := tests

# Toolchain
CC     := gcc
CFLAGS := -Wall -Wextra -O2 -I$(SHARED_DIR)

# Binaries
CLI_BIN  := $(CLI_DIR)/fwctl
CLI_SRCS := $(CLI_DIR)/fwctl.c $(CLI_DIR)/parser.c $(CLI_DIR)/netlink_client.c $(CLI_DIR)/watcher.c

.PHONY: all kernel cli clean install remove log test test-watch

# Default target builds both kernel module and user-space CLI
all: kernel cli

# Build kernel module
kernel:
	$(MAKE) -C $(KERNEL_DIR)

# Build CLI tool
cli: $(CLI_BIN)

$(CLI_BIN): $(CLI_SRCS)
	$(CC) $(CFLAGS) $(CLI_SRCS) -o $@

# Kernel management helper rules
install:
	sudo insmod $(KERNEL_DIR)/netfilter_firewall.ko

remove:
	sudo rmmod netfilter_firewall || true

log:
	sudo dmesg | tail -n 20

# Rule-filtering test suite (existing)
test:
	sudo $(TESTS_DIR)/test_firewall.sh

# Watch-mode test: verifies fwctl picks up live config changes
test-watch:
	sudo $(TESTS_DIR)/test_watch.sh

# Cleanup all generated files
clean:
	$(MAKE) -C $(KERNEL_DIR) clean
	rm -f $(CLI_BIN) $(CLI_DIR)/test_watcher $(CLI_DIR)/test_parser
