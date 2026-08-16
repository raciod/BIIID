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

#define NETLINK_USER 31
#define MAX_PAYLOAD sizeof(struct fw_rule)
 
int main(void)
{
 // struct fw_rule_node rule_list = parse_rules(DEFAULT_);   
 // printf("%s\n", DEFAULT_CONFIG_PATH);

 struct fw_rule_node *rule_list = parse_rules(DEFAULT_CONFIG_PATH);
 struct fw_rule_node *head = rule_list;
 if(rule_list == NULL) {
    printf("error in parse_rule\n");
    return 1;
 }
 // else printf("parsed successfuly\n");


  int nl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USER);
  if (nl_fd < 0) {
    perror("socket");
    free_rule_list(head);
    return 1;
  }
 
  struct sockaddr_nl src_addr = {0};
  src_addr.nl_family = AF_NETLINK;
  src_addr.nl_pid = getpid();
 
  if (bind(nl_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0) {
  perror("bind");
    close(nl_fd);
    free_rule_list(head);
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
    free_rule_list(head);
    return 1;
  }
    
  memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
  nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
  nlh->nlmsg_pid = getpid();
  nlh->nlmsg_flags = 0;


 // print_rule_list(rule_list);

  while(rule_list){
    struct fw_rule rule = {
        .action    = rule_list->rule.action,
        .src_ip    = rule_list->rule.src_ip,
        .dst_ip    = rule_list->rule.dst_ip,
        .protocol  = rule_list->rule.protocol,
        .port      = rule_list->rule.port,
        .direction = rule_list->rule.direction,
    };
 
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
 
    // Send the Data
    if (sendmsg(nl_fd, &msg, 0) < 0) {
        perror("sendmsg");
        free(nlh);
        close(nl_fd);
        free_rule_list(head);
        return 1;
    }
 
    rule_list = rule_list->next;
  }
  free(nlh);
  close(nl_fd);
  free_rule_list(head);


    return 0;
}


