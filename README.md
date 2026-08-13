# BIIID

**BIIID** is a custom Linux firewall currently in early development. It uses a Linux kernel module leveraging the Netfilter framework for packet filtering, combined with a user-space daemon communicating over Netlink sockets.

> **Status:** Active Development / Proof of Concept. Not intended for production use.

---

## Overview

The core objective of BIIID is to build a modern, multi-tier firewall system that combines low-level C kernel performance with a high-level C++ management layer.

Currently, the project contains a working kernel proof-of-concept that registers Netfilter hooks (`LOCAL_IN` / `LOCAL_OUT`), listens on a Netlink socket (`NETLINK_USER 31`), and filters IPv4 packets based on rules sent from user-space.

---

## Development Roadmap

### Phase 1: Kernel Module Core (C)

* [x] Initial Netfilter hook integration (`NF_INET_LOCAL_IN` & `NF_INET_LOCAL_OUT`).


* [x] Netlink IPC for dynamic rule injection from user-space.


* [x] Automated test suite (`test_firewall.sh`).


* [ ] Implement linked lists (`<linux/list.h>`) in kernel space to support multiple simultaneous rules instead of a single active rule.
* [ ] Add packet drop counters and two-way Netlink status reporting.

### Phase 2: User-Space Daemon & DSL (C++)

* [ ] Build a human-readable config parser for an English-like syntax:
- Block incoming traffic from a specific host
```text
BLOCK IN SRC 192.168.1.100
```

- Block HTTP traffic
```text
BLOCK OUT PROTO TCP PORT 80
```

- Block incoming UDP traffic
```text
BLOCK IN PROTO UDP
```


```text
BLOCK IN SRC 192.168.1.100
BLOCK OUT PROTO TCP PORT 80
```


* [ ] Implement a user-space daemon using `inotify` to watch `/etc/biiid/firewall.conf` for live changes.
* [ ] Connect the C++ parser to the C Netlink sender bridge (`extern "C"`).

### Phase 3: Interactive TUI

* [ ] Build a Terminal User Interface (TUI) for interactive rule management and log viewing.
* [ ] Integrate TUI file-saving with the background daemon pipeline.
