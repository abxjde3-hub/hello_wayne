#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/skbuff.h>
#include <linux/net.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne");
MODULE_DESCRIPTION("Projeto 6 - WAYNE NET - Conta pacotes via netfilter");

static unsigned long total_pkts;
static unsigned long tcp_pkts;
static unsigned long udp_pkts;
static unsigned long icmp_pkts;
static unsigned long other_pkts;
static __be32 last_ip;

static unsigned int wayne_net_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph;
    if (!skb)
        return NF_ACCEPT;
    iph = ip_hdr(skb);
    if (!iph)
        return NF_ACCEPT;
    total_pkts++;
    last_ip = iph->saddr;
    if (iph->protocol == IPPROTO_TCP)
        tcp_pkts++;
    else if (iph->protocol == IPPROTO_UDP)
        udp_pkts++;
    else if (iph->protocol == IPPROTO_ICMP)
        icmp_pkts++;
    else
        other_pkts++;
    return NF_ACCEPT;
}

static struct nf_hook_ops wayne_net_ops = {
    .hook = wayne_net_hook,
    .pf = NFPROTO_IPV4,
    .hooknum = NF_INET_PRE_ROUTING,
    .priority = NF_IP_PRI_FIRST,
};

static int wayne_net_show(struct seq_file *m, void *v)
{
    seq_printf(m, "=== WAYNE NET 6.0 ===\n");
    seq_printf(m, "Total de pacotes: %lu\n", total_pkts);
    seq_printf(m, "TCP: %lu\n", tcp_pkts);
    seq_printf(m, "UDP: %lu\n", udp_pkts);
    seq_printf(m, "ICMP (ping): %lu\n", icmp_pkts);
    seq_printf(m, "Outros: %lu\n", other_pkts);
    seq_printf(m, "Ultimo IP origem: %pI4\n", &last_ip);
    seq_printf(m, "Status: HOOK ATIVO em PRE_ROUTING\n");
    return 0;
}

static int wayne_net_open(struct inode *inode, struct file *file)
{
    return single_open(file, wayne_net_show, NULL);
}

static const struct proc_ops wayne_net_proc_ops = {
    .proc_open = wayne_net_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init wayne_net_init(void)
{
    int ret;
    total_pkts = 0;
    tcp_pkts = 0;
    udp_pkts = 0;
    icmp_pkts = 0;
    other_pkts = 0;
    last_ip = 0;
    proc_create("wayne_net", 0, NULL, &wayne_net_proc_ops);
    ret = nf_register_net_hook(&init_net, &wayne_net_ops);
    if (ret) {
        remove_proc_entry("wayne_net", NULL);
        pr_err("WAYNE NET: falhou registrar hook %d\n", ret);
        return ret;
    }
    pr_info("WAYNE NET carregado - hook em PRE_ROUTING\n");
    return 0;
}

static void __exit wayne_net_exit(void)
{
    nf_unregister_net_hook(&init_net, &wayne_net_ops);
    remove_proc_entry("wayne_net", NULL);
    pr_info("WAYNE NET removido - total %lu pacotes\n", total_pkts);
}

module_init(wayne_net_init);
module_exit(wayne_net_exit);
