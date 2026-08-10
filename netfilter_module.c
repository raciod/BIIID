#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>

/* Pointer to hold the hook options */
static struct nf_hook_ops netfilter_ops;

/* Hook function for packet logging  */
static unsigned int packet_logger(void *priv, 
		struct sk_buff *skb, 
		const struct nf_hook_state *state){
	
	struct iphdr *ip_header;
	struct tcphdr *tcp_header;

	/* Extract IP header  */
	ip_header = ip_hdr(skb);
	if (!ip_header) return NF_ACCEPT;

	/* Check for TCP protocol  */
	if(ip_header->protocol == IPPROTO_TCP){
		tcp_header =tcp_hdr(skb);
		pr_info("Packet logged: SRC=%pI4, DST=%pI4, SPORT=%u, DPORT=%u\n", 
				&ip_header->saddr, &ip_header->daddr,
				ntohs(tcp_header->source), ntohs(tcp_header->dest));
	}

	/* Accept the packet */
      return NF_ACCEPT;
}

/* Module initialization */
static int __init net_filter_init(void){
	netfilter_ops.hook = packet_logger;
	netfilter_ops.pf = PF_INET;
	netfilter_ops.hooknum = NF_INET_PRE_ROUTING;
    	netfilter_ops.priority = NF_IP_PRI_FIRST;

	nf_register_net_hook(&init_net, &netfilter_ops);
	pr_info("Netfilter module loaded\n");
	return 0;
}

/* Module cleanup  */
static void __exit netfilter_exit(void){
	nf_unregister_net_hook(&init_net, &netfilter_ops);
	pr_info("Netfilter module unloaded\n");
}

module_init(net_filter_init);
module_exit(netfilter_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("raciod");
MODULE_DESCRIPTION("Basic Netfilter Module for Packet Logging");



