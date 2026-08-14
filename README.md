# BIIID

BIIID is a custom Linux firewall built as a kernel module using the Netfilter framework, combined with a user-space CLI tool (`fwctl`) that communicates with the kernel over a Netlink socket.

Status: active development / proof of concept. Not intended for production use.

## Overview

BIIID combines a kernel module that does the actual packet filtering with a user-space tool that reads a rule config file and pushes those rules into the kernel dynamically, without needing to recompile the module to change behavior.

The whole project is written in C, both in the kernel and in user-space, so the filtering logic, the config parser, and the Netlink communication all share one toolchain and one rule struct.

## How it works

`fwctl` reads `biiid.conf`, converts each line into a `struct fw_rule`, and sends it to the kernel module over a Netlink socket (family `NETLINK_USER`, number 31). The kernel module never parses text itself — it only receives this fixed binary struct and applies it. All the parsing and config handling stays in user-space, where bugs are easier to deal with; the kernel module's job is just to look at packets and decide accept or drop.

```c
struct fw_rule {
    __be32 src_ip;
    __be32 dst_ip;
    __u8   protocol;
    __u16  port;
    __u8   direction;   // FW_DIR_IN / FW_DIR_OUT
};
```

The module hooks into Netfilter at `NF_INET_LOCAL_IN` and `NF_INET_LOCAL_OUT`, so it sees traffic coming into and going out of the machine.

## Rule syntax

`fwctl` reads a column-based config file, one rule per line:

```
# Action   Protocol   IP_Address       Port   Direction
DROP       TCP        192.168.1.100    80     OUT
DROP       ANY        192.168.1.100    ANY    IN
DROP       TCP        ANY              22     IN
ACCEPT     ANY        ANY              ANY    ANY
```

- Action: `DROP` or `ACCEPT`
- Protocol: `TCP`, `UDP`, `ICMP`, or `ANY`
- IP_Address: a single address, interpreted as source or destination depending on Direction, or `ANY` as a wildcard
- Port: a port number, or `ANY`
- Direction: `IN` or `OUT`

## Project structure

- `kernel/` — the kernel module itself: Netfilter hooks, the Netlink receiver, and the filtering logic (`netfilter_firewall.c` / `.h`)
- `fwctl/` — the user-space CLI: reads `biiid.conf` and pushes rules to the kernel over Netlink (`fwctl.c`, `parser.c` / `.h`, `netlink_client.c` / `.h`)
- `shared/` — `fw_rule.h`, the struct and constants shared between the kernel module and `fwctl`
- `tests/` — `test_firewall.sh`, which builds, loads the module, generates test traffic, checks the kernel log, and unloads
- `config/` — example rule file, `biiid.conf`

## Build and run

You'll need kernel headers matching your currently running kernel.

Build and load the kernel module:
```
cd kernel
make
sudo insmod netfilter_firewall.ko
```

Build and run fwctl:
```
cd fwctl
make
./fwctl --config ../config/biiid.conf
```

Check the kernel log:
```
sudo dmesg | tail
```

Unload the module:
```
sudo rmmod netfilter_firewall
```

## A note on testing safely

This hooks into `LOCAL_IN` and `LOCAL_OUT`, which means it can intercept all traffic on the machine, including SSH. A bad rule can cut off network access immediately. Test in a VM you can snapshot and roll back, or on a machine you have physical access to rather than over a remote session. If something goes wrong, `sudo rmmod netfilter_firewall` removes the module and restores normal networking right away.

## Roadmap

Done so far:
- Netfilter hooks for LOCAL_IN and LOCAL_OUT
- Filtering by IP, protocol, and port
- Netlink pipeline between the kernel module and user-space
- fwctl sends a rule to the kernel over Netlink
- A basic automated test suite

Still to do:
- fwctl's config parser for biiid.conf's column syntax
- An in-kernel rule list (using linux/list.h) to support more than one active rule at a time
- Packet drop counters and status reporting back over Netlink
- A watch mode that reloads the config live using inotify
- An interactive terminal UI for managing rules and viewing logs

## License

See [LICENSE](./LICENSE).
