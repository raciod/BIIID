# BIIID

BIIID is a custom Linux firewall built as a kernel module using the Netfilter framework, combined with a user-space CLI tool (`fwctl`) that communicates with the kernel over a Netlink socket.

Status: active development / proof of concept. Not intended for production use.

## Overview

BIIID combines a kernel module that does the actual packet filtering with a user-space tool that reads a rule config file and pushes those rules into the kernel dynamically, without needing to recompile the module to change behavior.

The whole project is written in C, both in the kernel and in user-space, so the filtering logic, the config parser, and the Netlink communication all share one toolchain and one rule struct.

## How it works

`fwctl` reads `biiid.conf`, converts each line into a `struct fw_rule`, and sends one Netlink message per rule to the kernel module (family `NETLINK_USER`, number 31). The kernel module never parses text itself — it only receives this fixed binary struct and stores it. All parsing and config handling stays in user-space; the kernel module's job is to hold the current set of rules and decide, per packet, whether to accept or drop it.

```c
struct fw_rule {
    __u8   action;      // FW_ACTION_DROP / FW_ACTION_ACCEPT (parsed, not yet enforced -- see below)
    __be32 src_ip;
    __be32 dst_ip;
    __u8   protocol;
    __u16  port;
    __u8   direction;   // FW_DIR_IN / FW_DIR_OUT
};
```

Each rule the kernel receives is stored in an in-kernel linked list (`linux/list.h`), so more than one rule can be active at once. The module hooks into Netfilter at `NF_INET_LOCAL_IN` and `NF_INET_LOCAL_OUT`. On every packet, it walks the full rule list and drops the packet if any rule matches; if nothing matches, the packet is accepted.

**Known gap:** the `action` field is parsed from the config file and stored in each rule, but `firewall()` doesn't check it yet -- every match currently drops, regardless of whether the rule says `DROP` or `ACCEPT`. Real `ACCEPT` support is on the roadmap below.

## Rule syntax

`fwctl` reads a column-based config file, one rule per line:

```
# Action    Field       Value             Direction
DROP        SRC_IP      192.168.1.100     IN
DROP        DST_IP      192.168.1.100     OUT
DROP        PORT        80                OUT
DROP        PROTO       TCP               IN
```

- Action: `DROP` or `ACCEPT` (see known gap above)
- Field: `SRC_IP`, `DST_IP`, `PORT`, or `PROTO` -- each rule targets exactly one field, the rest are wildcarded
- Value: an IP address, a port number, or a protocol name (`TCP`, `UDP`, `ICMP`)
- Direction: `IN` or `OUT`

## Project structure

- `kernel/` -- the kernel module: Netfilter hooks, the Netlink receiver, the in-kernel rule list, and the filtering logic (`netfilter_firewall.c` / `.h`)
- `fwctl/` -- the user-space CLI: parses `biiid.conf` and pushes rules to the kernel over Netlink (`fwctl.c`, `parser.c` / `.h`)
- `shared/` -- `fw_rule.h`, the struct and constants shared between the kernel module and `fwctl`
- `tests/` -- `test_firewall.sh`; currently reflects an earlier single-rule design and needs updating for the multi-rule list (see roadmap)
- `config/` -- example rule file, `biiid.conf`

## Build and run

You'll need kernel headers matching your currently running kernel.

Build everything:
```
make
```

Load the kernel module:
```
sudo make install
```

Push the rules from `config/biiid.conf` into the kernel:
```
./fwctl/fwctl
```

Check the kernel log:
```
sudo make log
```

Unload the module:
```
sudo make remove
```

## A note on testing safely

This hooks into `LOCAL_IN` and `LOCAL_OUT`, which means it can intercept all traffic on the machine, including SSH. A bad rule can cut off network access immediately. Test in a VM you can snapshot and roll back, or on a machine you have physical access to rather than over a remote session. If something goes wrong, `sudo rmmod netfilter_firewall` removes the module and restores normal networking right away.

Also worth knowing: a broad rule like `DROP PROTO TCP` blocks all TCP traffic unconditionally, which will also mask any more specific TCP-based rule (like a `PORT` rule) from ever being the one that fires, since the protocol rule matches first. Keep this in mind when testing more than one rule at a time.

## Roadmap

Done so far:
- Netfilter hooks for LOCAL_IN and LOCAL_OUT
- Filtering by IP, protocol, and port
- Netlink pipeline between the kernel module and user-space
- fwctl parses biiid.conf and sends every rule to the kernel over Netlink
- In-kernel rule list (`linux/list.h`) -- multiple simultaneous rules, not just one
- A basic automated test suite (needs updating for multi-rule behavior)

Still to do:
- Enforce the `action` field so `ACCEPT` actually works, not just `DROP`
- Update `tests/test_firewall.sh` to reflect multi-rule behavior instead of the old single-rule design
- Kernel-to-userspace notifications: push a message back over Netlink when a packet is dropped, instead of only logging to `dmesg`
- A watch mode that reloads the config live using inotify
- An interactive terminal UI for managing rules and viewing live drop events

## License

See [LICENSE](./LICENSE).
