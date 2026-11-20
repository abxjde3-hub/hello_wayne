#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

#define WAYNE_DMA_SIZE 4096

static struct input_dev *wayne_dma_dev;
static struct kobject *wayne_dma_kobj;
static void *wayne_dma_src;
static void *wayne_dma_dst;
static int wayne_dma_press;
static int wayne_dma_copies;
static int wayne_dma_bytes;

static void wayne_dma_do_copy(void)
{
    memcpy(wayne_dma_dst, wayne_dma_src, WAYNE_DMA_SIZE);
    wayne_dma_copies++;
    wayne_dma_bytes += WAYNE_DMA_SIZE;
    pr_info("WAYNE DMA 40.0: copia %d - %d bytes - DMA simulado - src %p dst %p\n", wayne_dma_copies, wayne_dma_bytes, wayne_dma_src, wayne_dma_dst);
}

static ssize_t wayne_dma_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "Wayne Dma Paw - press %d - copias %d - bytes %d\n", wayne_dma_press, wayne_dma_copies, wayne_dma_bytes);
}

static ssize_t wayne_dma_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_dma_press++;
    wayne_dma_do_copy();
    input_report_key(wayne_dma_dev, KEY_M, 1);
    input_sync(wayne_dma_dev);
    input_report_key(wayne_dma_dev, KEY_M, 0);
    input_sync(wayne_dma_dev);
    pr_info("WAYNE DMA 40.0: aperto %d - copias %d - KEY_M - Wayne Dma Paw\n", wayne_dma_press, wayne_dma_copies);
    return count;
}

static struct kobj_attribute wayne_dma_attr = __ATTR(press, 0664, wayne_dma_show, wayne_dma_store);

static int __init wayne_dma_init(void)
{
    int err;
    pr_info("WAYNE DMA 40.0: iniciando o dma\n");

    wayne_dma_dev = input_allocate_device();
    if (!wayne_dma_dev)
        return -ENOMEM;

    wayne_dma_dev->name = "Wayne Dma Paw";
    wayne_dma_dev->phys = "wayne/dma0";
    wayne_dma_dev->id.bustype = BUS_VIRTUAL;
    wayne_dma_dev->id.vendor = 0x1234;
    wayne_dma_dev->id.product = 0x0040;
    wayne_dma_dev->id.version = 0x4000;

    __set_bit(EV_KEY, wayne_dma_dev->evbit);
    __set_bit(EV_SYN, wayne_dma_dev->evbit);
    __set_bit(KEY_M, wayne_dma_dev->keybit);

    err = input_register_device(wayne_dma_dev);
    if (err) {
        input_free_device(wayne_dma_dev);
        return err;
    }

    wayne_dma_src = kzalloc(WAYNE_DMA_SIZE, GFP_KERNEL);
    if (!wayne_dma_src) {
        input_unregister_device(wayne_dma_dev);
        return -ENOMEM;
    }

    wayne_dma_dst = kzalloc(WAYNE_DMA_SIZE, GFP_KERNEL);
    if (!wayne_dma_dst) {
        kfree(wayne_dma_src);
        input_unregister_device(wayne_dma_dev);
        return -ENOMEM;
    }

    memset(wayne_dma_src, 0x57, WAYNE_DMA_SIZE);
    memset(wayne_dma_dst, 0, WAYNE_DMA_SIZE);

    wayne_dma_press = 0;
    wayne_dma_copies = 0;
    wayne_dma_bytes = 0;

    wayne_dma_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_dma_kobj) {
        kfree(wayne_dma_dst);
        kfree(wayne_dma_src);
        input_unregister_device(wayne_dma_dev);
        return -ENOMEM;
    }

    err = sysfs_create_file(wayne_dma_kobj, &wayne_dma_attr.attr);
    if (err) {
        kobject_put(wayne_dma_kobj);
        kfree(wayne_dma_dst);
        kfree(wayne_dma_src);
        input_unregister_device(wayne_dma_dev);
        return err;
    }

    pr_info("WAYNE DMA 40.0: dma criado - 4096 bytes - Wayne Dma Paw - DMA simulado no WSL\n");
    return 0;
}

static void __exit wayne_dma_exit(void)
{
    if (wayne_dma_kobj) {
        sysfs_remove_file(wayne_dma_kobj, &wayne_dma_attr.attr);
        kobject_put(wayne_dma_kobj);
    }
    kfree(wayne_dma_dst);
    kfree(wayne_dma_src);
    input_unregister_device(wayne_dma_dev);
    pr_info("WAYNE DMA 40.0: dma removido - press %d - copias %d - bytes %d\n", wayne_dma_press, wayne_dma_copies, wayne_dma_bytes);
}

module_init(wayne_dma_init);
module_exit(wayne_dma_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 40 - WAYNE DMA - memoria rapida");
