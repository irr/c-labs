/*
  AUTHOR: Ivan Ribeiro Rocha (ivan.ribeiro@gmail.com)

  BUILD: make clean && make
  INSTALL: insmod cs_DADDR.ko && dmesg
  REMOVE: rmmod cs_DADDR && dmesg
*/

#define DRIVER_AUTHOR "Ivan Ribeiro Rocha"
#define DRIVER_DESC   "cs_DADDR (checksum)"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <net/tcp.h>

#define DEBUG 

static struct nf_hook_ops _nfho;

unsigned int hook_func(unsigned int hooknum,
                       struct sk_buff *skb,
                       const struct net_device *in,
                       const struct net_device *out,
                       int (*okfn)(struct sk_buff *))
{
    struct iphdr *ip_header = 0;
    struct tcphdr *tcp_header = 0;
    int tcplen;

#ifdef DEBUG  
    __sum16 ip_old, tcp_old;
#endif

    ip_header = (struct iphdr *) skb_network_header(skb);

    if (ip_header->tos == 0) {
        return NF_ACCEPT;
    }

    if (ip_header->protocol != IPPROTO_TCP) {
        return NF_ACCEPT;
    }

    tcp_header = (struct tcphdr *)(skb->data + (ip_header->ihl << 2));
      
#ifdef DEBUG  
    ip_old = ip_header->check;
    tcp_old = tcp_header->check; 
#endif

    if (skb_is_nonlinear(skb))
        skb_linearize(skb);

    tcplen = (skb->len - (ip_header->ihl << 2));

    tcp_header->check = 0;
    tcp_header->check = tcp_v4_check(tcplen,
                                     ip_header->saddr,
                                     ip_header->daddr,
                                     csum_partial((char*) tcp_header, tcplen, 0));

    ip_header->check = 0;
    ip_header->check = ip_fast_csum((u8 *)ip_header, ip_header->ihl);     

#ifdef DEBUG  
    printk(KERN_DEBUG "cs_DADDR: %pI4:%d -> %pI4:%d (tos:0x%x) - ip:0x%x-0x%x tcp:0x%x-0x%x\n", 
                &ip_header->saddr, ntohs(tcp_header->source), 
                &ip_header->daddr, ntohs(tcp_header->dest), 
                ip_header->tos, ip_old, htons(ip_header->check), htons(tcp_old), htons(tcp_header->check));
#endif

    return NF_ACCEPT;
}

int init_module(void)
{
    _nfho.hook = hook_func;
    _nfho.hooknum = 4;
    _nfho.pf = PF_INET;
    _nfho.priority = NF_IP_PRI_FIRST;
    nf_register_hook(&_nfho);

    printk(KERN_INFO "cs_DADDR: init_module() called\n");
    return 0;
}

void cleanup_module(void)
{
    nf_unregister_hook(&_nfho);
    printk(KERN_INFO "cs_DADDR: cleanup_module() called\n");
}

MODULE_LICENSE("GPLv2");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
