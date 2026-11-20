#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/spi/spi.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_spi_dev;
static struct kobject *wayne_spi_kobj;
static int wayne_spi_press;
static int wayne_spi_transfers;
static int wayne_spi_bytes;
static u8 wayne_spi_tx[16];
static u8 wayne_spi_rx[16];

static void wayne_spi_do_transfer(void)
{
    int i;
    for (i = 0; i < 16; i++) {
        wayne_spi_tx[i] = 0x50 + i + wayne_spi_transfers;
        wayne_spi_rx[i] = wayne_spi_tx[i] ^ 0xFF;
    }
    wayne_spi_transfers++;
    wayne_spi_bytes += 16;
    pr_info("WAYNE SPI 45.0: transferencia %d - %d bytes - tx 0x%02x rx 0x%02x - SPI simulado\n", wayne_spi_transfers, wayne_spi_bytes, wayne_spi_tx[0], wayne_spi_rx[0]);
}

static ssize_t wayne_spi_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "Wayne Spi Paw - SPI bus 0 cs 0 - press %d - transfers %d - bytes %d\n", wayne_spi_press, wayne_spi_transfers, wayne_spi_bytes);
}

static ssize_t wayne_spi_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_spi_press++;
    wayne_spi_do_transfer();
    input_report_key(wayne_spi_dev, KEY_S, 1);
    input_sync(wayne_spi_dev);
    input_report_key(wayne_spi_dev, KEY_S, 0);
    input_sync(wayne_spi_dev);
    pr_info("WAYNE SPI 45.0: aperto %d - transfers %d - KEY_S - Wayne Spi Paw - SPI bus 0 cs 0\n", wayne_spi_press, wayne_spi_transfers);
    return count;
}

static struct kobj_attribute wayne_spi_attr = __ATTR(press, 0664, wayne_spi_show, wayne_spi_store);

static int __init wayne_spi_init(void)
{
    int err;
    pr_info("WAYNE SPI 45.0: iniciando o spi\n");

    wayne_spi_dev = input_allocate_device();
    if (!wayne_spi_dev)
        return -ENOMEM;

    wayne_spi_dev->name = "Wayne Spi Paw";
    wayne_spi_dev->phys = "wayne/spi0";
    wayne_spi_dev->id.bustype = BUS_SPI;
    wayne_spi_dev->id.vendor = 0x1234;
    wayne_spi_dev->id.product = 0x0045;
    wayne_spi_dev->id.version = 0x4500;

    __set_bit(EV_KEY, wayne_spi_dev->evbit);
    __set_bit(EV_SYN, wayne_spi_dev->evbit);
    __set_bit(KEY_S, wayne_spi_dev->keybit);

    err = input_register_device(wayne_spi_dev);
    if (err) {
        input_free_device(wayne_spi_dev);
        return err;
    }

    wayne_spi_press = 0;
    wayne_spi_transfers = 0;
    wayne_spi_bytes = 0;
    memset(wayne_spi_tx, 0, sizeof(wayne_spi_tx));
    memset(wayne_spi_rx, 0, sizeof(wayne_spi_rx));

    wayne_spi_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_spi_kobj) {
        input_unregister_device(wayne_spi_dev);
        return -ENOMEM;
    }

    err = sysfs_create_file(wayne_spi_kobj, &wayne_spi_attr.attr);
    if (err) {
        kobject_put(wayne_spi_kobj);
        input_unregister_device(wayne_spi_dev);
        return err;
    }

    pr_info("WAYNE SPI 45.0: spi criado - bus 0 cs 0 - Wayne Spi Paw - SPI simulado no WSL\n");
    return 0;
}

static void __exit wayne_spi_exit(void)
{
    if (wayne_spi_kobj) {
        sysfs_remove_file(wayne_spi_kobj, &wayne_spi_attr.attr);
        kobject_put(wayne_spi_kobj);
    }
    input_unregister_device(wayne_spi_dev);
    pr_info("WAYNE SPI 45.0: spi removido - press %d - transfers %d - bytes %d\n", wayne_spi_press, wayne_spi_transfers, wayne_spi_bytes);
}

module_init(wayne_spi_init);
module_exit(wayne_spi_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 45 - WAYNE SPI - protocolo rapido");
