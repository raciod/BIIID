#ifndef FW_H
#define FW_H

#include "../shared/fw_rule.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/list.h>

#define NETLINK_USER 31

struct fw_rule_entry {
  struct fw_rule rule;
  struct list_head list;
};

#endif
