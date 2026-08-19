#include "netlink_client.h"
#include "../shared/fw_rule.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/netlink.h>

int init_netlink_socket(void)
{
    int nl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (nl_fd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_nl src_addr = {0};
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();

    if (bind(nl_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0) {
        perror("bind");
        close(nl_fd);
        return -1;
    }

    return nl_fd;
}

int send_rules_fd(int nl_fd, struct fw_rule_node *rules)
{
    if (rules == NULL) {
        fprintf(stderr, "send_rules_fd: rules list is NULL\n");
        return 1;
    }

    if (nl_fd < 0) {
        fprintf(stderr, "send_rules_fd: invalid socket descriptor\n");
        return 1;
    }

    struct sockaddr_nl dest_addr = {0};
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; // 0 = Kernel
    dest_addr.nl_groups = 0;

    struct nlmsghdr *nlh = malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if (!nlh) {
        perror("malloc");
        return 1;
    }

    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    while (rules) {
        struct fw_rule rule = {
            .action    = rules->rule.action,
            .src_ip    = rules->rule.src_ip,
            .dst_ip    = rules->rule.dst_ip,
            .protocol  = rules->rule.protocol,
            .port      = rules->rule.port,
            .direction = rules->rule.direction,
        };

        memcpy(NLMSG_DATA(nlh), &rule, sizeof(struct fw_rule));

        struct iovec iov = {
            .iov_base = nlh,
            .iov_len  = nlh->nlmsg_len,
        };
        struct msghdr msg = {
            .msg_name    = &dest_addr,
            .msg_namelen = sizeof(dest_addr),
            .msg_iov     = &iov,
            .msg_iovlen  = 1,
        };

        if (sendmsg(nl_fd, &msg, 0) < 0) {
            perror("sendmsg");
            free(nlh);
            return 1;
        }

        rules = rules->next;
    }

    free(nlh);
    return 0;
}

int send_rules(struct fw_rule_node *rules)
{
    int nl_fd = init_netlink_socket();
    if (nl_fd < 0)
        return 1;

    int res = send_rules_fd(nl_fd, rules);
    close(nl_fd);
    return res;
}

int receive_drop_notification(int nl_fd)
{
    char buffer[1024];
    struct sockaddr_nl nladdr;
    socklen_t addrlen = sizeof(nladdr);

    ssize_t len = recvfrom(nl_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&nladdr, &addrlen);
    if (len < 0) {
        perror("recvfrom");
        return -1;
    }

    struct nlmsghdr *nlh = (struct nlmsghdr *)buffer;
    if (!NLMSG_OK(nlh, (size_t)len)) {
        fprintf(stderr, "Netlink message corrupted\n");
        return -1;
    }

    struct fw_rule *rule = (struct fw_rule *)NLMSG_DATA(nlh);

    char src_buf[INET_ADDRSTRLEN];
    char dst_buf[INET_ADDRSTRLEN];
    struct in_addr src = { .s_addr = rule->src_ip };
    struct in_addr dst = { .s_addr = rule->dst_ip };

    inet_ntop(AF_INET, &src, src_buf, sizeof(src_buf));
    inet_ntop(AF_INET, &dst, dst_buf, sizeof(dst_buf));

    printf("[KERNEL DROP ALERT] src=%s dst=%s proto=%u port=%u dir=%s\n",
           rule->src_ip ? src_buf : "ANY",
           rule->dst_ip ? dst_buf : "ANY",
           (unsigned int)rule->protocol,
           (unsigned int)rule->port,
           rule->direction == FW_DIR_IN ? "IN" : "OUT");

    return 0;
}
