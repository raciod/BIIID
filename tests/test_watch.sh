#!/usr/bin/env bash
# tests/test_watch.sh
# Verifies that fwctl's watch mode picks up config file changes and
# pushes the new rules to the kernel module.

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT"

CONFIG_FILE="config/biiid.conf"
FWCTL_BIN="fwctl/fwctl"

echo -e "${YELLOW}=== Watch Mode Test ===${NC}\n"

if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Error: run as root (sudo tests/test_watch.sh)${NC}"
    exit 1
fi

# Build kernel module + fwctl CLI
echo "[+] Building project..."
make >/dev/null || { echo -e "${RED}Build failed${NC}"; exit 1; }

# Reload kernel module cleanly
echo "[+] Reloading kernel module..."
rmmod netfilter_firewall 2>/dev/null
insmod kernel/netfilter_firewall.ko || { echo -e "${RED}insmod failed${NC}"; exit 1; }

# Clear dmesg buffer
dmesg -c > /dev/null

# Start fwctl in watch mode in the background
echo "[+] Starting fwctl in watch mode on $CONFIG_FILE..."
"$FWCTL_BIN" --watch &
WATCHER_PID=$!
sleep 1

# Trigger a config change
echo "[+] Modifying $CONFIG_FILE to trigger a reparse..."
echo "# test_watch.sh trigger $(date +%s)" >> "$CONFIG_FILE"
sleep 1

# Stop fwctl watcher
kill "$WATCHER_PID" 2>/dev/null
wait "$WATCHER_PID" 2>/dev/null

echo -e "\n--- dmesg since module reload ---"
dmesg

echo
RECEIVED_COUNT=$(dmesg | grep -c "Netlink: received rule")

if [ "$RECEIVED_COUNT" -ge 2 ]; then
    echo -e "${GREEN}PASS${NC}: kernel received $RECEIVED_COUNT rule(s) after the config change was detected."
else
    echo -e "${RED}FAIL${NC}: expected received rules after reparse, got $RECEIVED_COUNT."
    echo "Check: is the kernel module loaded with a working Netlink receiver?"
fi
