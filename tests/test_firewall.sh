#!/usr/bin/env bash
set -e

CONF_PATH="config/biiid.conf"

echo "=== 1. Building Project ==="
make clean
make

echo "=== 2. Loading Kernel Module ==="
sudo insmod kernel/netfilter_firewall.ko

echo "=== 3. Writing Multi-Rule Test Config ==="
cat << 'EOF' > "$CONF_PATH"
DROP DST_IP 127.0.0.2 IN
DROP PORT 9999 OUT
EOF

echo "=== 4. Pushing Rules to Kernel via CLI ==="
./fwctl/fwctl

echo "=== 5. Checking Kernel Log Output ==="
sudo dmesg | tail -n 10

echo "=== 6. Testing Module Unload Safety ==="
sudo rmmod netfilter_firewall

if sudo dmesg | tail -n 10 | grep -qi "unloaded"; then
    echo "[PASS] Module unloaded cleanly."
else
    echo "[WARN] Module unloaded, but no exit log found in dmesg."
fi
