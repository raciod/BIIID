#include "netfilter_firewall.h"

static struct nf_hook_ops firewall_hook_in;
static struct nf_hook_ops firewall_hook_out;

/*  Filter function based on PORT, PROTOCOL, IP SCR, IP DEST */
static unsigned int firewall(void *priv,
                    struct sk_buff *skb,
                    const struct nf_hook_state *state) {
  struct iphdr *ip_header;
  struct tcphdr *tcp_header;

  ip_header = ip_hdr(skb);
  if (!ip_header) return NF_ACCEPT;

  if (ip_header->saddr == BLOCKED_IP_SCR) {
    pr_info("Dropped: blocked source IP\n");
    return NF_DROP;
  }

  if (ip_header->daddr == BLOCKED_IP_DEST) {
    pr_info("Dropped: blocked dest IP\n");
    return NF_DROP;
  }
  
  if (ip_header->protocol == BLOCKED_PROTO) {
    pr_info("Dropped: blocked protocol\n");
    return NF_DROP;
  }

  if (ip_header->protocol == IPPROTO_TCP) {
    tcp_header = tcp_hdr(skb);
    if (ntohs(tcp_header->dest) == BLOCKED_PORT) {
        pr_info("Dropped: blocked port\n");
        return NF_DROP;
    }
  }

  return NF_ACCEPT;
}

/* init module */
static int __init firewall_init(void){
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

/*  exit module */
static void __exit firewall_exit(void) {
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

