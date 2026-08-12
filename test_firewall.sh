#!/bin/bash
# test_firewall.sh — build, (re)load, and test the netfilter firewall module.
# Usage: sudo ./test_firewall.sh [target_ip]
#
# Run as root (needed for insmod/rmmod).

set -e

MODULE_NAME="netfilter_firewall"
TARGET_IP="${1:-192.168.1.100}"   # override: ./test_firewall.sh <ip>

if [ "$EUID" -ne 0 ]; then
    echo "Please run as root (sudo ./test_firewall.sh)"
    exit 1
fi

echo "== Building module =="
make clean >/dev/null
make

echo
echo "== Removing old module if loaded =="
if lsmod | grep -q "$MODULE_NAME"; then
    rmmod "$MODULE_NAME"
    echo "Old module removed."
else
    echo "No existing module loaded."
fi

echo
echo "== Loading module =="
insmod "${MODULE_NAME}.ko"
sleep 1

echo
echo "== Generating test traffic to $TARGET_IP =="
ping -c 2 -W 1 "$TARGET_IP" > /dev/null 2>&1 || true
curl -s -m 2 "http://$TARGET_IP" > /dev/null 2>&1 || true

echo
echo "== Kernel log (last 20 lines) =="
dmesg | tail -n 20

echo
echo "== Done. Module is still loaded. =="
echo "   To unload manually: sudo rmmod $MODULE_NAME"
