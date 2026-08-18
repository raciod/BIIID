#ifndef FW_RULE_H
#define FW_RULE_H

#include <linux/types.h>
#define FW_DIR_IN  0
#define FW_DIR_OUT 1

#define FW_ACTION_DROP   0
#define FW_ACTION_ACCEPT 1

#define DEFAULT_CONFIG_PATH "/home/raciod/Data/Projects/Fire-Wall/config/biiid.conf"

#define NETLINK_USER 31

struct fw_rule {
    __u8   action;
    __be32 src_ip;
    __be32 dst_ip;
    __u8   protocol;
    __u16  port;
    __u8   direction;
};

#define MAX_PAYLOAD sizeof(struct fw_rule)
// int send_rules(struct fw_rule_node *rules);

#endif
