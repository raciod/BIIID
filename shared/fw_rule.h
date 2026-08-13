#ifndef FW_RULE_H
#define FW_RULE_H

#include <linux/netlink.h>

#define FW_DIR_IN  0
#define FW_DIR_OUT 1

struct fw_rule {
    __be32 src_ip;
    __be32 dst_ip;
    __u8   protocol;
    __u16  port;
    __u8   direction;
};

#endif
