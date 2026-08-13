#include "fw_rule.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define NETLINK_USER 31
#define MAX_PAYLOAD sizeof(struct fw_rule)
 
int main(void)
{
    struct fw_rule rule = {
        .src_ip    = htonl(0xC0A80164),
        .dst_ip    = htonl(0xC0A80164),
        .protocol  = IPPROTO_TCP,
        .port      = 80,
        .direction = FW_DIR_OUT,
    };
 
    int nl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (nl_fd < 0) {
        perror("socket");
        return 1;
    }
 
    struct sockaddr_nl src_addr = {0};
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
 
    if (bind(nl_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0) {
        perror("bind");
        close(nl_fd);
        return 1;
    }
 
    struct sockaddr_nl dest_addr = {0};
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;      // 0 = kernel
    dest_addr.nl_groups = 0;
 
    struct nlmsghdr *nlh = malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if (!nlh) {
        perror("malloc");
        close(nl_fd);
        return 1;
    }
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;
 
    memcpy(NLMSG_DATA(nlh), &rule, sizeof(struct fw_rule));
 
    struct iovec iov = {
        .iov_base = nlh,
        .iov_len = nlh->nlmsg_len,
    };
    struct msghdr msg = {
        .msg_name = &dest_addr,
        .msg_namelen = sizeof(dest_addr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
    };
 
    printf("Sending rule: src=%s proto=TCP port=%u dir=%s\n",
           "192.168.1.100", rule.port,
           rule.direction == FW_DIR_IN ? "IN" : "OUT");
 
    if (sendmsg(nl_fd, &msg, 0) < 0) {
        perror("sendmsg");
        free(nlh);
        close(nl_fd);
        return 1;
    }
 
    free(nlh);
    close(nl_fd);
    return 0;
}


