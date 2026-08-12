#ifndef FW_H
#define FW_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#define BLOCKED_IP_SCR htonl(0xC0A80164)
#define BLOCKED_IP_DEST htonl(0xC0A80164) 
#define BLOCKED_PROTO IPPROTO_TCP
#define BLOCKED_PORT 80

#define FW_DIR_IN  0
#define FW_DIR_OUT 1


#endif
