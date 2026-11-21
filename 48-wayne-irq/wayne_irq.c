#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_irq_dev;
static struct kobject *wayne_irq_kobj;
static int wayne_irq_press;
static int wayne_irq_num;
static int wayne_irq_count;
static int wayne_irq_last;

static irqreturn_t wayne_irq_handler(int irq, void *dev_id)
{
    wayne_irq_count++;
    wayne_irq_last = irq;
    pr_info("WAYNE IRQ 48.0: interrupcao %d - irq %d - IRQ simulado\n", wayne_irq_count, irq);
    return IRQ_HANDLED;
}

static ssize_t wayne_irq_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "Wayne Irq Paw - IRQ %d - press %d - count %d - last %d\n", wayne_irq_num, wayne_irq_press, wayne_irq_count, wayne_irq_last);
}

static ssize_t wayne_irq_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_irq_press++;
    wayne_irq_handler(wayne_irq_num, NULL);
    input_report_key(wayne_irq_dev, KEY_I, 1);
    input_sync(wayne_irq_dev);
    input_report_key(wayne_irq_dev, KEY_I, 0);
    input_sync(wayne_irq_dev);
    pr_info("WAYNE IRQ 48.0: aperto %d - irq %d - count %d - KEY_I - Wayne Irq Paw - IRQ %d\n", wayne_irq_press, wayne_irq_num, wayne_irq_count, wayne_irq_num);
    return count;
}

static struct kobj_attribute wayne_irq_attr = __ATTR(press, 0664, wayne_irq_show, wayne_irq_store);

static int __init wayne_irq_init(void)
{
    int err;
    pr_info("WAYNE IRQ 48.0: iniciando o irq\n");

    wayne_irq_dev = input_allocate_device();
    if (!wayne_irq_dev)
        return -ENOMEM;

    wayne_irq_dev->name = "Wayne Irq Paw";
    wayne_irq_dev->phys = "wayne/irq0";
    wayne_irq_dev->id.bustype = BUS_VIRTUAL;
    wayne_irq_dev->id.vendor = 0x1234;
    wayne_irq_dev->id.product = 0x0048;
    wayne_irq_dev->id.version = 0x4800;

    __set_bit(EV_KEY, wayne_irq_dev->evbit);
    __set_bit(EV_SYN, wayne_irq_dev->evbit);
    __set_bit(KEY_I, wayne_irq_dev->keybit);

    err = input_register_device(wayne_irq_dev);
    if (err) {
        input_free_device(wayne_irq_dev);
        return err;
    }

    wayne_irq_press = 0;
    wayne_irq_num = 48;
    wayne_irq_count = 0;
    wayne_irq_last = 0;

    wayne_irq_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_irq_kobj) {
        input_unregister_device(wayne_irq_dev);
        return -ENOMEM;
    }

    err = sysfs_create_file(wayne_irq_kobj, &wayne_irq_attr.attr);
    if (err) {
        kobject_put(wayne_irq_kobj);
        input_unregister_device(wayne_irq_dev);
        return err;
    }

    pr_info("WAYNE IRQ 48.0: irq criado - IRQ 48 - Wayne Irq Paw - IRQ simulado no WSL\n");
    return 0;
}

static void __exit wayne_irq_exit(void)
{
    if (wayne_irq_kobj) {
        sysfs_remove_file(wayne_irq_kobj, &wayne_irq_attr.attr);
        kobject_put(wayne_irq_kobj);
    }
    input_unregister_device(wayne_irq_dev);
    pr_info("WAYNE IRQ 48.0: irq removido - press %d - count %d - last %d\n", wayne_irq_press, wayne_irq_count, wayne_irq_last);
}

module_init(wayne_irq_init);
module_exit(wayne_irq_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 48 - WAYNE IRQ - interrupcao");
