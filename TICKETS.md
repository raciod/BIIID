# Fire-Wall — Daily Tickets

15 working days (3 weeks × 5 days), ~1–3 hrs/day. Each ticket = one day's focused goal with a concrete "done" condition. Check them off as you go — if a ticket spills into a second day, that's fine, just shift the rest.

---

## Week 1 — Static filtering + netlink foundation

- [ ] **Day 1 — Rule struct + hardcoded filtering**
  Define `struct fw_rule` in a shared header. Hardcode a small array of 2-3 rules directly in the kernel module (e.g. block one IP, block one port). Extend `packet_logger` to loop over the array and return `NF_DROP` on match, `NF_ACCEPT` otherwise.
  **Done when:** `insmod`, then a blocked `ping`/`curl` from your VM actually fails, and an unblocked one succeeds. Confirmed in `dmesg`.

- [ ] **Day 2 — Cleanup + safety habits**
  Add a "panic switch": a way to force-unload cleanly (`rmmod`) even mid-test, and double check `PRE_ROUTING` isn't blocking your own VM's SSH/loopback by accident. Write a short `TESTING.md` note: your exact VM setup, snapshot/rollback steps.
  **Done when:** you can break something on purpose and recover in under 1 minute using your own notes.

- [ ] **Day 3 — Netlink kernel-side skeleton**
  Research + implement `netlink_kernel_create` and a receive callback in the module. Have it just `pr_info` whatever raw bytes arrive — no rule logic yet.
  **Done when:** a manually-crafted userspace test message (even a throwaway script) shows up in `dmesg` when sent to your netlink socket.

- [ ] **Day 4 — Netlink: struct fw_rule over the wire**
  Update the kernel receive callback to interpret incoming bytes as a real `struct fw_rule` (not raw junk) and print its fields.
  **Done when:** you can send one fabricated `struct fw_rule` from a quick userspace test program and see the exact fields logged correctly in `dmesg`.

- [ ] **Day 5 — Netlink replaces hardcoded array**
  Module stores received rules in a small in-kernel list/array instead of the Day 1 hardcoded one. Filtering logic now reads from this dynamic list.
  **Done when:** module starts with zero rules (all traffic allowed), and after sending one rule via your test program, that traffic is correctly blocked — no recompile needed.

---

## Week 2 — fwctl CLI + YAML

- [ ] **Day 6 — fwctl skeleton**
  Start the userspace `fwctl` program. Set up its `AF_NETLINK` socket (matching the kernel side), and hardcode one rule in C to send — replacing your Day 4/5 throwaway test script with a real, reusable tool.
  **Done when:** `./fwctl` sends a hardcoded rule and the module applies it correctly.

- [ ] **Day 7 — fwctl CLI arguments**
  Add argument parsing (`getopt` or manual) so you can run e.g. `./fwctl block --proto tcp --dst-port 22 --dir in` instead of hardcoding.
  **Done when:** you can add/test at least 2 different rules purely via CLI flags, no recompiling `fwctl` itself.

- [ ] **Day 8 — fwctl: list & remove rules**
  Add `./fwctl list` (kernel sends current rules back over netlink) and `./fwctl remove <id>`. This requires the kernel side to support a "query" and "delete" message type, not just "add."
  **Done when:** you can add 3 rules, list them, remove one, and list again to confirm.

- [ ] **Day 9 — YAML parsing (userspace only)**
  Pick a small YAML parsing approach (a minimal C library, or a deliberately simplified custom format if you want zero dependencies). Parse a `config.yaml` into an array of `struct fw_rule` in `fwctl` — don't send to kernel yet, just print the parsed result.
  **Done when:** a `config.yaml` with 3-4 rules prints correctly as parsed structs.

- [ ] **Day 10 — YAML → netlink, full pipeline**
  Wire Day 9's parsed rules into Day 6-8's netlink send logic: `./fwctl --config config.yaml` pushes every rule from the file into the kernel in one go.
  **Done when:** editing `config.yaml` and re-running `fwctl` changes live firewall behavior end-to-end, config file → kernel enforcement.

---

## Week 3 — TUI, stretch, polish

- [ ] **Day 11 — Logging you can actually read**
  Either extend netlink to support the kernel pushing "packet was blocked/allowed" events to userspace, or start simpler: have `fwctl` tail `dmesg` and reformat it nicely.
  **Done when:** you have a live, readable stream of decisions outside raw `dmesg`.

- [ ] **Day 12-13 — TUI dashboard**
  Build a small ncurses (or similar) dashboard: live feed of allow/block events, maybe a simple rule count/status view.
  **Done when:** you can watch live traffic get blocked/allowed in the TUI while generating test traffic in another terminal.

- [ ] **Day 14 — Stretch OR buffer day**
  If on schedule: attempt Stage 6 (simple SYN flood detection). If behind schedule: use this day to catch up on any earlier ticket that overflowed.

- [ ] **Day 15 — CV polish**
  Finalize README (architecture diagram, usage, safety notes — already mostly done). Record a short demo (GIF or video) showing `fwctl` blocking live traffic in real time. Final commit + tag (e.g. `v1.0`).
  **Done when:** a stranger could clone the repo, read the README, and understand what this project does and why, in under 2 minutes.
