#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_gp_dev;
static struct kobject *wayne_gp_kobj;
static int wayne_gp_value;
static int wayne_gp_count;

static void wayne_gp_fire(void)
{
    wayne_gp_count++;
    if (wayne_gp_value) {
        input_report_key(wayne_gp_dev, KEY_G, 1);
        input_sync(wayne_gp_dev);
        input_report_key(wayne_gp_dev, KEY_G, 0);
        input_sync(wayne_gp_dev);
        pr_info("WAYNE GPIO 32.0: GPIO ALTO - pino 1 - aperto %d - KEY_G - Wayne Gpio Paw\n", wayne_gp_count);
    } else {
        pr_info("WAYNE GPIO 32.0: GPIO BAIXO - pino 0 - aperto %d - Wayne Gpio Paw\n", wayne_gp_count);
    }
}

static ssize_t wayne_gp_value_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", wayne_gp_value);
}

static ssize_t wayne_gp_value_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int v;
    if (kstrtoint(buf, 10, &v) == 0) {
        wayne_gp_value = (v != 0);
        wayne_gp_fire();
    }
    return count;
}

static ssize_t wayne_gp_count_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "Wayne Gpio Paw - pino %d - apertou %d vezes\n", wayne_gp_value, wayne_gp_count);
}

static struct kobj_attribute wayne_gp_value_attr = __ATTR(value, 0664, wayne_gp_value_show, wayne_gp_value_store);
static struct kobj_attribute wayne_gp_count_attr = __ATTR(count, 0444, wayne_gp_count_show, NULL);

static int __init wayne_gp_init(void)
{
    int err;
    pr_info("WAYNE GPIO 32.0: iniciando o gpio\n");

    wayne_gp_dev = input_allocate_device();
    if (!wayne_gp_dev)
        return -ENOMEM;

    wayne_gp_dev->name = "Wayne Gpio Paw";
    wayne_gp_dev->phys = "wayne/gpio0";
    wayne_gp_dev->id.bustype = BUS_VIRTUAL;
    wayne_gp_dev->id.vendor = 0x1234;
    wayne_gp_dev->id.product = 0x0032;
    wayne_gp_dev->id.version = 0x3200;

    __set_bit(EV_KEY, wayne_gp_dev->evbit);
    __set_bit(EV_SYN, wayne_gp_dev->evbit);
    __set_bit(KEY_G, wayne_gp_dev->keybit);

    err = input_register_device(wayne_gp_dev);
    if (err) {
        input_free_device(wayne_gp_dev);
        return err;
    }

    wayne_gp_value = 0;
    wayne_gp_count = 0;

    wayne_gp_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (wayne_gp_kobj) {
        sysfs_create_file(wayne_gp_kobj, &wayne_gp_value_attr.attr);
        sysfs_create_file(wayne_gp_kobj, &wayne_gp_count_attr.attr);
    }

    pr_info("WAYNE GPIO 32.0: gpio criado - pino em /sys/kernel/wayne/value - Wayne Gpio Paw\n");
    return 0;
}

static void __exit wayne_gp_exit(void)
{
    if (wayne_gp_kobj) {
        sysfs_remove_file(wayne_gp_kobj, &wayne_gp_value_attr.attr);
        sysfs_remove_file(wayne_gp_kobj, &wayne_gp_count_attr.attr);
        kobject_put(wayne_gp_kobj);
    }
    input_unregister_device(wayne_gp_dev);
    pr_info("WAYNE GPIO 32.0: gpio removido - pino %d - apertou %d vezes\n", wayne_gp_value, wayne_gp_count);
}

module_init(wayne_gp_init);
module_exit(wayne_gp_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 32 - WAYNE GPIO - pino de hardware");
