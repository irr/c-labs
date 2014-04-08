/*
  AUTHOR: Ivan Ribeiro Rocha (ivan.ribeiro@gmail.com)

  BUILD: make clean && make
  INSTALL: sudo insmod test.ko && dmesg
  REMOVE: sudo rmmod test.ko && dmesg

  cd /usr/src/kernels/`uname -r`
  sb include/linux/skbuff.h
  sb include/linux/netfilter_ipv4.h
  sb include/linux/netfilter.h
  sb include/linux/ip.h
  sb include/linux/icmp.h
  sb include/linux/in.h

  cd /usr/include/
  sb linux/types.h

  sudo ln -s /path/to/test.ko /lib/modules/`uname -r`
  or
  sudo cp test.ko /lib/modules/`uname -r`/
  sudo depmod -a
  sudo modprobe test

  cd /etc/modprobe.d/
  echo "options test mask=10" > test.conf

  CENTOS/REDHAT:
  echo 'modprobe test' >> /etc/rc.modules
  chmod +x /etc/rc.modules

  make && sudo insmod test.ko && dmesg
  curl --connect-timeout 2 -v http://10.133.25.16/
  sudo rmmod test.ko; dmesg

  Staging test:
  10.133.25.16 -> 176494864
  make clean; make && insmod test.ko && dmesg && curl --connect-timeout 2 -v http://10.133.25.16/; echo; sudo rmmod test.ko; dmesg 
*/

#define DRIVER_AUTHOR "Ivan Ribeiro Rocha"
#define DRIVER_DESC   "Test Filter"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <net/ip.h>
#include <net/tcp.h>

#include <linux/string.h>

static struct nf_hook_ops _nfho1;
static struct nf_hook_ops _nfho2;

static long mask = 0;

/*
cat /sys/module/test/parameters/mask
...
S_IRUGO = read-only
S_IRUGO|S_IWUSR = read-write
*/

module_param(mask, long, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(mask, "network address bitmask");

unsigned int check_addrs(__be32 *saddr, __be32 *daddr) {
    unsigned int status = NF_ACCEPT;
    
    printk(KERN_INFO
           "test: ip-src %pI4 (%u) -> ip-dst %pI4 (%u) mask:%li (status:%s)\n",
           saddr, ntohl(*saddr), daddr, ntohl(*daddr), mask, (status == NF_ACCEPT) ? "ACCEPT" : "DROP");
    return status;
}

unsigned int hook_func_in(unsigned int hooknum,
                          struct sk_buff *skb,
                          const struct net_device *in,
                          const struct net_device *out,
                          int (*okfn)(struct sk_buff *))
{
    struct iphdr *ip_header = 0;
    struct tcphdr *tcp_header = 0;
    struct udphdr *udp_header = 0;
    struct icmphdr *icmp_header = 0;

    ip_header = (struct iphdr *) skb_network_header(skb);

    switch (ip_header->protocol) {
        case IPPROTO_UDP:
            udp_header = (struct udphdr *)(skb->data + (ip_header->ihl << 2));
            if (udp_header)
                printk(KERN_INFO "IN: test-udp: %pI4:%d --> %pI4:%d\n", &ip_header->saddr, ntohs(udp_header->source), 
                        &ip_header->daddr, ntohs(udp_header->dest));
            break;
        case IPPROTO_ICMP:
            icmp_header = (struct icmphdr *)(skb->data + (ip_header->ihl << 2));
            if (icmp_header)
                printk(KERN_INFO "IN: test-icmp: %pI4 --> %pI4 (type:%d, code:%d)\n", &ip_header->saddr, &ip_header->daddr, 
                        icmp_header->type, icmp_header->code);          
            break;
        case IPPROTO_TCP:
            tcp_header = (struct tcphdr *)(skb->data + (ip_header->ihl << 2));
            if (tcp_header)
                  printk(KERN_INFO "IN: test-tcp: %pI4:%d --> %pI4:%d\n", &ip_header->saddr, ntohs(tcp_header->source), 
                        &ip_header->daddr, ntohs(tcp_header->dest));
            break;
        }

    return check_addrs(&ip_header->saddr, &ip_header->daddr);
}

unsigned int hook_func_out(unsigned int hooknum,
                           struct sk_buff *skb,
                           const struct net_device *in,
                           const struct net_device *out,
                           int (*okfn)(struct sk_buff *))
{
    struct iphdr *ip_header = 0;
    struct tcphdr *tcp_header = 0;
    struct udphdr *udp_header = 0;
    struct icmphdr *icmp_header = 0;

    ip_header = (struct iphdr *) skb_network_header(skb);

    switch (ip_header->protocol) {
        case IPPROTO_UDP:
            udp_header = (struct udphdr *)(skb->data + (ip_header->ihl << 2));
            if (udp_header)
                printk(KERN_INFO "OUT: test-udp: %pI4:%d --> %pI4:%d\n", &ip_header->saddr, ntohs(udp_header->source), 
                    &ip_header->daddr, ntohs(udp_header->dest));
            break;
        case IPPROTO_ICMP:
            icmp_header = (struct icmphdr *)(skb->data + (ip_header->ihl << 2));
            if (icmp_header)
                printk(KERN_INFO "OUT: test-icmp: %pI4 --> %pI4 (type:%d, code:%d)\n", &ip_header->saddr, &ip_header->daddr, 
                    icmp_header->type, icmp_header->code);          
            break;
        case IPPROTO_TCP:
            tcp_header = (struct tcphdr *)(skb->data + (ip_header->ihl << 2));
            if ((tcp_header) && (ntohs(tcp_header->dest) == 8888)) {
                int payload_offset = ((ip_header->ihl << 2) + (tcp_header->doff << 2));
                unsigned int payload_length = (unsigned int) ntohs(ip_header->tot_len) - 
                                              ((ip_header->ihl << 2) + (tcp_header->doff << 2));
                if (payload_length) {
                    printk(KERN_INFO "OUT1: test-tcp: %pI4:%d --> %pI4:%d (TOS:%d)->%x-%x po:%d pl:%d\n", &ip_header->saddr, 
                        ntohs(tcp_header->source), &ip_header->daddr, ntohs(tcp_header->dest), 
                        ip_header->tos, htons(ip_header->check), htons(tcp_header->check), payload_offset, payload_length);
                
                    if(skb_is_nonlinear(skb))
                        skb_linearize(skb);

                    char *data = (char *)(skb->data + payload_offset);

                    if (strncmp(data, "irr", 3) == 0) {
                        strcpy(data, "ale");
                    }

                    int tcplen = (skb->len - (ip_header->ihl << 2));
                    tcp_header->check = 0;
                    tcp_header->check = tcp_v4_check(tcplen,
                                                     ip_header->saddr,
                                                     ip_header->daddr,
                                                     csum_partial((char*) tcp_header, tcplen, 0));

                    ip_header->tos = ip_header->tos + 1;

                    ip_header->check = 0;
                    ip_header->check = ip_fast_csum((u8 *)ip_header, ip_header->ihl);     

                    printk(KERN_INFO "OUT2: test-tcp: %pI4:%d --> %pI4:%d (TOS:%d)->%x-%x po:%d pl:%d => %s\n", &ip_header->saddr, 
                        ntohs(tcp_header->source), &ip_header->daddr, ntohs(tcp_header->dest), 
                        ip_header->tos, htons(ip_header->check), htons(tcp_header->check), payload_offset, payload_length, data);
                }
            }
            break;
    }

    return check_addrs(&ip_header->saddr, &ip_header->daddr);
}

int init_module(void)
{
    _nfho1.hook = hook_func_in;
    _nfho1.hooknum = 0;
    _nfho1.pf = PF_INET;
    _nfho1.priority = NF_IP_PRI_FIRST;
    /*nf_register_hook(&_nfho1);*/

    _nfho2.hook = hook_func_out;
    _nfho2.hooknum = 4;
    _nfho2.pf = PF_INET;
    _nfho2.priority = NF_IP_PRI_FIRST;
    nf_register_hook(&_nfho2);

    printk(KERN_INFO "test: init_module() called\n");
    return 0;
}

void cleanup_module(void)
{
    /*nf_unregister_hook(&_nfho1);*/
    nf_unregister_hook(&_nfho2);
    printk(KERN_INFO "test: cleanup_module() called\n");
}

MODULE_LICENSE("GPLv2");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
