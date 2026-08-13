#include "netfilter_firewall.h"
#include "../shared/fw_rule.h"

static struct nf_hook_ops firewall_hook_in;
static struct nf_hook_ops firewall_hook_out;

static struct fw_rule active_rule;
static struct sock *nl_sock = NULL;

/* Receive data */
static void netlink_recv_msg(struct sk_buff *skb) {
  struct nlmsghdr *nlh;
  struct fw_rule *rule;

  nlh = (struct nlmsghdr *)skb->data;
  rule = (struct fw_rule *)nlmsg_data(nlh);

  printk(KERN_INFO "Netlink: received rule - scr=%pI4 dst=%pI4 proto=%u port=%u dir=%u\n", 
         &rule->src_ip, &rule->dst_ip, (unsigned int)rule->protocol, 
         (unsigned int)rule->port, (unsigned int)rule->direction);
  
  active_rule = *rule;
}

static struct netlink_kernel_cfg cfg = {
  .input = netlink_recv_msg,
};

/* Filter function based on PORT, PROTOCOL, IP SRC, IP DEST */
static unsigned int firewall(void *priv,
                             struct sk_buff *skb,
                             const struct nf_hook_state *state) {
  struct iphdr *ip_header;
  struct tcphdr *tcp_header;

  ip_header = ip_hdr(skb);
  if (!ip_header) return NF_ACCEPT;

  // 1. Check Source IP
  if (active_rule.src_ip != 0 && ip_header->saddr == active_rule.src_ip) {
    pr_info("Dropped: blocked source IP\n");
    return NF_DROP;
  }

  // 2. Check Destination IP
  if (active_rule.dst_ip != 0 && ip_header->daddr == active_rule.dst_ip) {
    pr_info("Dropped: blocked dest IP\n");
    return NF_DROP;
  }

  // 3. Check Protocol and Port
  if (active_rule.protocol != 0 && ip_header->protocol == active_rule.protocol) {
    if (active_rule.port != 0) {
      if (ip_header->protocol == IPPROTO_TCP) {
        tcp_header = tcp_hdr(skb);
        if (tcp_header && ntohs(tcp_header->dest) == active_rule.port) {
          pr_info("Dropped: blocked TCP port %d\n", active_rule.port);
          return NF_DROP;
        }
      }
    } else {
      pr_info("Dropped: blocked protocol %d\n", active_rule.protocol);
      return NF_DROP;
    }
  }

  return NF_ACCEPT;
}

/* init module */
static int __init firewall_init(void) {
  nl_sock = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
  if (!nl_sock) {
    printk(KERN_ERR "Netlink: failed to create socket\n");
    return -ENOMEM;
  }
  
  firewall_hook_in.hook = firewall;
  firewall_hook_in.pf = PF_INET;
  firewall_hook_in.hooknum = NF_INET_LOCAL_IN;
  firewall_hook_in.priority = NF_IP_PRI_FIRST;

  nf_register_net_hook(&init_net, &firewall_hook_in);
  pr_info("FireWall IN module loaded\n");

  firewall_hook_out.hook = firewall;
  firewall_hook_out.pf = PF_INET;
  firewall_hook_out.hooknum = NF_INET_LOCAL_OUT;
  firewall_hook_out.priority = NF_IP_PRI_FIRST;

  nf_register_net_hook(&init_net, &firewall_hook_out);
  pr_info("FireWall OUT module loaded\n");

  return 0;
}

/* exit module */
static void __exit firewall_exit(void) {
  netlink_kernel_release(nl_sock);
  printk(KERN_INFO "Netlink: module unloaded\n");

  nf_unregister_net_hook(&init_net, &firewall_hook_in);
  pr_info("Firewall IN Module Unloaded\n");

  nf_unregister_net_hook(&init_net, &firewall_hook_out);
  pr_info("Firewall OUT Module Unloaded\n");
}

module_init(firewall_init);
module_exit(firewall_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("raciod");
MODULE_DESCRIPTION("Custom Firewall Using Netfilter");
