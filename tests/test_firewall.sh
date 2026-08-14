#!/usr/bin/env bash

# Script: test_firewall.sh
# Purpose: Comprehensive test suite for netfilter_firewall kernel module

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Automatically resolve the project root directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Switch working directory to project root so relative paths work cleanly
cd "$PROJECT_ROOT"

echo -e "${YELLOW}=== Comprehensive Netfilter Firewall Test Suite ===${NC}\n"

# Ensure root privileges
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Error: This script must be run as root (e.g., sudo ./tests/test_firewall.sh)${NC}"
    exit 1
fi

# Load kernel module if not currently loaded
if ! lsmod | grep -q "netfilter_firewall"; then
    echo "[+] Loading netfilter_firewall.ko module..."
    if [ -f "kernel/netfilter_firewall.ko" ]; then
        insmod kernel/netfilter_firewall.ko
    else
        echo "[+] Kernel module binary not found. Building project..."
        make kernel
        if [ -f "kernel/netfilter_firewall.ko" ]; then
            insmod kernel/netfilter_firewall.ko
        else
            echo -e "${RED}Error: Failed to build kernel/netfilter_firewall.ko.${NC}"
            exit 1
        fi
    fi
fi

# Compile and run the user-space controller to send the rule
echo "[+] Compiling fwctl user-space tool..."
gcc cli/fwctl.c -I shared -o cli/fwctl

if [ ! -f "cli/fwctl" ]; then
    echo -e "${RED}Error: Failed to compile cli/fwctl.c${NC}"
    exit 1
fi

echo "[+] Sending firewall rule via Netlink..."
./cli/fwctl
sleep 1 # Give the kernel a moment to process the Netlink message

TEST_NAMES=()
TEST_EXPECTED=()
TEST_ACTUAL=()
TEST_STATUS=()

PASS_COUNT=0
FAIL_COUNT=0

run_test() {
    local test_name="$1"
    local expected="$2"
    shift 2
    local cmd=("$@")

    if "${cmd[@]}" >/dev/null 2>&1; then
        res="ACCEPT"
    else
        res="DROP"
    fi

    if [ "$res" == "$expected" ]; then
        status="PASS"
        ((PASS_COUNT++))
    else
        status="FAIL"
        ((FAIL_COUNT++))
    fi

    TEST_NAMES+=("$test_name")
    TEST_EXPECTED+=("$expected")
    TEST_ACTUAL+=("$res")
    TEST_STATUS+=("$status")
}

# Flush previous kernel logs for clean dmesg output
dmesg -c > /dev/null

echo "Executing test cases..."

# 1. Allowed ICMP (Ping)
run_test "Allowed ICMP Ping (127.0.0.1)" "ACCEPT" ping -c 1 -W 1 127.0.0.1

# 2. Blocked Destination IP (192.168.1.100)
run_test "Blocked Dest IP (192.168.1.100)" "DROP" ping -c 1 -W 1 192.168.1.100

# 3. Blocked Source IP (192.168.1.100)
ip addr add 192.168.1.100/32 dev lo 2>/dev/null
run_test "Blocked Source IP (192.168.1.100)" "DROP" ping -I 192.168.1.100 -c 1 -W 1 127.0.0.1
ip addr del 192.168.1.100/32 dev lo 2>/dev/null

# 4. Blocked TCP Port (80)
run_test "Blocked TCP Port 80 (HTTP)" "DROP" nc -z -w 1 1.1.1.1 80

# 5. TCP Port (443 - HTTPS)
run_test "Allowed TCP Port 443 (HTTPS)" "ACCEPT" nc -z -w 1 1.1.1.1 443

# 6. Protocol Block Check (UDP)
run_test "UDP Traffic Check (8.8.8.8:53)" "ACCEPT" nc -u -z -w 1 8.8.8.8 53

echo -e "\n--- Kernel Log Output (dmesg) ---"
dmesg | grep "Dropped:" || echo "No drop logs found in dmesg."

# Print Summary Table
echo -e "\n${YELLOW}=== Test Results Summary ===${NC}\n"

printf "+----------------------------------+----------+--------+--------+\n"
printf "| %-32s | %-8s | %-6s | %-6s |\n" "Test Description" "Expected" "Actual" "Status"
printf "+----------------------------------+----------+--------+--------+\n"

for i in "${!TEST_NAMES[@]}"; do
    st="${TEST_STATUS[$i]}"
    if [ "$st" == "PASS" ]; then
        color="${GREEN}"
    else
        color="${RED}"
    fi

    printf "| %-32s | %-8s | %-6s | ${color}%-6s${NC} |\n" \
        "${TEST_NAMES[$i]}" \
        "${TEST_EXPECTED[$i]}" \
        "${TEST_ACTUAL[$i]}" \
        "${TEST_STATUS[$i]}"
done

printf "+----------------------------------+----------+--------+--------+\n"
echo -e "Total Passed: ${GREEN}${PASS_COUNT}${NC} | Total Failed: ${RED}${FAIL_COUNT}${NC}\n"
