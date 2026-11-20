#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct net_device *wayne_net_dev;
static struct kobject *wayne_net_kobj;
static int wayne_net_tx;
static int wayne_net_rx;

static netdev_tx_t wayne_net_xmit(struct sk_buff *skb, struct net_device *dev)
{
    wayne_net_tx++;
    dev->stats.tx_packets++;
    dev->stats.tx_bytes += skb->len;
    pr_info("WAYNE NET 35.0: pacote enviado %d - %u bytes - Wayne Net Paw\n", wayne_net_tx, skb->len);
    dev_kfree_skb(skb);
    return NETDEV_TX_OK;
}

static int wayne_net_open(struct net_device *dev)
{
    netif_start_queue(dev);
    pr_info("WAYNE NET 35.0: interface wayne0 aberta - pronta!\n");
    return 0;
}

static int wayne_net_stop(struct net_device *dev)
{
    netif_stop_queue(dev);
    pr_info("WAYNE NET 35.0: interface wayne0 fechada\n");
    return 0;
}

static const struct net_device_ops wayne_net_ops = {
    .ndo_open = wayne_net_open,
    .ndo_stop = wayne_net_stop,
    .ndo_start_xmit = wayne_net_xmit,
};

static void wayne_net_setup(struct net_device *dev)
{
    ether_setup(dev);
    dev->netdev_ops = &wayne_net_ops;
    dev->flags |= IFF_NOARP;
    dev->features |= NETIF_F_HW_CSUM;
}

static ssize_t wayne_net_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "Wayne Net Paw - wayne0 - TX %d - RX %d\n", wayne_net_tx, wayne_net_rx);
}

static ssize_t wayne_net_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    struct sk_buff *skb;
    skb = dev_alloc_skb(64);
    if (skb) {
        skb_put(skb, 64);
        wayne_net_xmit(skb, wayne_net_dev);
    }
    return count;
}

static struct kobj_attribute wayne_net_attr = __ATTR(press, 0664, wayne_net_show, wayne_net_store);

static int __init wayne_net_init(void)
{
    int err;
    pr_info("WAYNE NET 35.0: iniciando o net\n");

    wayne_net_dev = alloc_netdev(0, "wayne0", NET_NAME_UNKNOWN, wayne_net_setup);
    if (!wayne_net_dev)
        return -ENOMEM;

    memcpy(wayne_net_dev->dev_addr, "\x02\x00\x00\x00\x00\x35", ETH_ALEN);

    err = register_netdev(wayne_net_dev);
    if (err) {
        free_netdev(wayne_net_dev);
        return err;
    }

    wayne_net_tx = 0;
    wayne_net_rx = 0;

    wayne_net_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (wayne_net_kobj)
        sysfs_create_file(wayne_net_kobj, &wayne_net_attr.attr);

    pr_info("WAYNE NET 35.0: net criado - interface wayne0 - Wayne Net Paw\n");
    return 0;
}

static void __exit wayne_net_exit(void)
{
    if (wayne_net_kobj) {
        sysfs_remove_file(wayne_net_kobj, &wayne_net_attr.attr);
        kobject_put(wayne_net_kobj);
    }
    unregister_netdev(wayne_net_dev);
    free_netdev(wayne_net_dev);
    pr_info("WAYNE NET 35.0: net removido - TX %d - RX %d\n", wayne_net_tx, wayne_net_rx);
}

module_init(wayne_net_init);
module_exit(wayne_net_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 35 - WAYNE NET - rede virtual");
