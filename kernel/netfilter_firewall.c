#include "netfilter_firewall.h"
#include "../shared/fw_rule.h"
#include <linux/in.h>
#include <linux/netfilter.h>

static struct nf_hook_ops firewall_hook_in;
static struct nf_hook_ops firewall_hook_out;

static struct list_head rule_list_head;

// static struct fw_rule active_rule;
static struct sock *nl_sock = NULL;

/* Receive data */
static void netlink_recv_msg(struct sk_buff *skb) {
  struct nlmsghdr *nlh;
  struct fw_rule *rule;
  struct fw_rule_entry *rule_entry; 

  rule_entry = (struct fw_rule_entry*)kmalloc(sizeof(struct fw_rule_entry), GFP_KERNEL);
  if(rule_entry == NULL){
    printk(KERN_ERR "Netlink: kmalloc(rule_entry) failed\n");
    return;
  }

  nlh = (struct nlmsghdr *)skb->data;
  rule = (struct fw_rule *)nlmsg_data(nlh);

  printk(KERN_INFO "Netlink: received rule - scr=%pI4 dst=%pI4 proto=%u port=%u dir=%u\n", 
         &rule->src_ip, &rule->dst_ip, (unsigned int)rule->protocol, 
         (unsigned int)rule->port, (unsigned int)rule->direction);
  
  rule_entry->rule = *rule;
  list_add(&rule_entry->list, &rule_list_head); // add to the list
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
    struct udphdr *udp_header;
    struct fw_rule_entry *pos;

    ip_header = ip_hdr(skb);
    if (!ip_header) return NF_ACCEPT;

    list_for_each_entry(pos, &rule_list_head, list) {  

        // 1. Direction Filter
        if (pos->rule.direction == FW_DIR_IN && state->hook != NF_INET_LOCAL_IN)
            continue;
        if (pos->rule.direction == FW_DIR_OUT && state->hook != NF_INET_LOCAL_OUT)
            continue;

        // 2. Source IP Check
        if (pos->rule.src_ip != 0 && ip_header->saddr != pos->rule.src_ip)
            continue;

        // 3. Destination IP Check
        if (pos->rule.dst_ip != 0 && ip_header->daddr != pos->rule.dst_ip)
            continue;

        // 4. Protocol Check
        if (pos->rule.protocol != 0 && ip_header->protocol != pos->rule.protocol)
            continue;

        // 5. Port Check (TCP & UDP)
        if (pos->rule.port != 0) {
            if (ip_header->protocol == IPPROTO_TCP) {
                tcp_header = tcp_hdr(skb);
                if (!tcp_header || (ntohs(tcp_header->dest) != pos->rule.port && 
                                   ntohs(tcp_header->source) != pos->rule.port))
                    continue;
            } else if (ip_header->protocol == IPPROTO_UDP) {
                udp_header = udp_hdr(skb);
                if (!udp_header || (ntohs(udp_header->dest) != pos->rule.port && 
                                   ntohs(udp_header->source) != pos->rule.port))
                    continue;
            } else {
                // Port rule defined, but packet is neither TCP nor UDP
                continue;
            }
        }

        // If a packet survived all field checks without skipping, it's a full match!
        pr_info("BIIID: Dropped packet matching rule\n");
        return NF_DROP;
    }

    return NF_ACCEPT;
}

/* init module */
static int __init firewall_init(void) {

  INIT_LIST_HEAD(&rule_list_head);

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

  struct fw_rule_entry *pos, *n;
  list_for_each_entry_safe(pos, n, &rule_list_head, list) {
    list_del(&pos->list);
    kfree(pos);
  }
}

module_init(firewall_init);
module_exit(firewall_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("raciod");
MODULE_DESCRIPTION("Custom Firewall Using Netfilter");

