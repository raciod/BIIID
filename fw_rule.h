#ifndef FW_RULE_H
#define FW_RULE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/netlink.h>


struct fw_rule {
    __be32 src_ip;
    __be32 dst_ip;
    __u8   protocol;
    __u16  port;
    __u8   direction;
};


#endif
