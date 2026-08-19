#ifndef NLC_H
#define NLC_H

#include "parser.h"

int init_netlink_socket(void);
int send_rules_fd(int nl_fd, struct fw_rule_node *rules);
int send_rules(struct fw_rule_node *rules);
int receive_drop_notification(int nl_fd);

#endif
