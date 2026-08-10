# Fire-Wall

A Linux firewall implemented as a kernel module using the **Netfilter** framework. Intercepts and filters packets directly in the kernel network stack (`NF_INET_PRE_ROUTING` hook), with rules configured via YAML and pushed into the kernel over a **netlink** socket.

## Architecture

```
config.yaml → [fwctl: userspace CLI] → parses YAML, builds struct fw_rule[]
                                              │
                                     netlink socket (AF_NETLINK)
                                              │
                                              ▼
                              [kernel module: netfilter hook]
                          NIC → PRE_ROUTING → rule match → NF_ACCEPT / NF_DROP
```

**Design principle:** the kernel module never parses text or YAML. It only ever receives a fixed binary `struct fw_rule` over netlink and applies it. All parsing, validation, and config management live in userspace, where bugs are cheap and libc is available.

```c
struct fw_rule {
    __be32 src_ip, dst_ip;   // 0 = wildcard/any
    __u16  src_port, dst_port;
    __u8   protocol;          // TCP / UDP / ICMP
    __u8   direction;         // IN / OUT
    __u8   action;            // ALLOW / BLOCK
};
```

## Build & run

**Requirements:** kernel headers matching your running kernel (`/usr/src/kernels/$(uname -r)` or equivalent for your distro).

**Compile:**
```
make
```

**Load the module:**
```
sudo insmod netfilter_module.ko
```

**Check the kernel log:**
```
sudo dmesg | tail
```

**Unload the module:**
```
sudo rmmod netfilter_module
```

## ⚠️ Testing safety

This hooks `PRE_ROUTING`, meaning it can intercept **all incoming traffic, including SSH**. A bad rule (e.g. an unconditional `NF_DROP`) can lock you out instantly if you're testing over a remote/SSH session.

- Test in a **VM** with a local console/snapshot you can roll back, or
- Test on **bare metal with physical access**, not over SSH.

## Roadmap

- [x] Kernel-side packet logging via Netfilter
- [ ] Static in-kernel rule filtering (allow/block)
- [ ] Netlink pipeline between kernel module and userspace
- [ ] `fwctl` CLI tool to manage rules
- [ ] YAML-based configuration
- [ ] Live TUI dashboard for traffic decisions
- [ ] Stretch: basic stateful/SYN-flood detection

See [`TICKETS.md`](./TICKETS.md) for the detailed day-by-day plan.

## Author

raciod
