#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/interrupt.h>

static struct input_dev *wayne_irq_dev;
static struct timer_list wayne_irq_timer;
static struct kobject *wayne_irq_kobj;
static int wayne_irq_count;
static int wayne_irq_handled;

static irqreturn_t wayne_irq_handler(int irq, void *dev_id)
{
    wayne_irq_handled++;
    input_report_key(wayne_irq_dev, KEY_I, 1);
    input_sync(wayne_irq_dev);
    input_report_key(wayne_irq_dev, KEY_I, 0);
    input_sync(wayne_irq_dev);
    pr_info("WAYNE IRQ 31.0: IRQ atendida %d - KEY_I - Wayne Irq Paw\n", wayne_irq_handled);
    return IRQ_HANDLED;
}

static void wayne_irq_timer_fn(struct timer_list *t)
{
    wayne_irq_count++;
    pr_info("WAYNE IRQ 31.0: hardware apertou %d - chamando IRQ...\n", wayne_irq_count);
    wayne_irq_handler(0, NULL);
    mod_timer(&wayne_irq_timer, jiffies + msecs_to_jiffies(3000));
}

static ssize_t wayne_irq_sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "Wayne Irq Paw - hardware apertou %d - IRQ atendida %d\n", wayne_irq_count, wayne_irq_handled);
}

static ssize_t wayne_irq_sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_irq_count++;
    pr_info("WAYNE IRQ 31.0: voce apertou de fora %d - chamando IRQ...\n", wayne_irq_count);
    wayne_irq_handler(0, NULL);
    return count;
}

static struct kobj_attribute wayne_irq_attr = __ATTR(press, 0664, wayne_irq_sysfs_show, wayne_irq_sysfs_store);

static int __init wayne_irq_init(void)
{
    int err;
    pr_info("WAYNE IRQ 31.0: iniciando o irq\n");

    wayne_irq_dev = input_allocate_device();
    if (!wayne_irq_dev)
        return -ENOMEM;

    wayne_irq_dev->name = "Wayne Irq Paw";
    wayne_irq_dev->phys = "wayne/irq0";
    wayne_irq_dev->id.bustype = BUS_VIRTUAL;
    wayne_irq_dev->id.vendor = 0x1234;
    wayne_irq_dev->id.product = 0x0031;
    wayne_irq_dev->id.version = 0x3100;

    __set_bit(EV_KEY, wayne_irq_dev->evbit);
    __set_bit(EV_SYN, wayne_irq_dev->evbit);
    __set_bit(KEY_I, wayne_irq_dev->keybit);

    err = input_register_device(wayne_irq_dev);
    if (err) {
        input_free_device(wayne_irq_dev);
        return err;
    }

    wayne_irq_count = 0;
    wayne_irq_handled = 0;

    wayne_irq_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (wayne_irq_kobj)
        sysfs_create_file(wayne_irq_kobj, &wayne_irq_attr.attr);

    timer_setup(&wayne_irq_timer, wayne_irq_timer_fn, 0);
    mod_timer(&wayne_irq_timer, jiffies + msecs_to_jiffies(3000));

    pr_info("WAYNE IRQ 31.0: irq ligado - hardware simulado a cada 3s - Wayne Irq Paw\n");
    return 0;
}

static void __exit wayne_irq_exit(void)
{
    timer_delete_sync(&wayne_irq_timer);
    if (wayne_irq_kobj) {
        sysfs_remove_file(wayne_irq_kobj, &wayne_irq_attr.attr);
        kobject_put(wayne_irq_kobj);
    }
    input_unregister_device(wayne_irq_dev);
    pr_info("WAYNE IRQ 31.0: irq removido - hardware apertou %d - IRQ atendida %d\n", wayne_irq_count, wayne_irq_handled);
}

module_init(wayne_irq_init);
module_exit(wayne_irq_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 31 - WAYNE IRQ - interrupcao simulada");
