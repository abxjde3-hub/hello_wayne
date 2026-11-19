#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_sysfs_dev;
static struct kobject *wayne_kobj;
static int wayne_press_count;

static ssize_t wayne_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "Wayne Sysfs Paw - apertou %d vezes\n", wayne_press_count);
}

static ssize_t wayne_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_press_count++;
    input_report_key(wayne_sysfs_dev, KEY_A, 1);
    input_sync(wayne_sysfs_dev);
    input_report_key(wayne_sysfs_dev, KEY_A, 0);
    input_sync(wayne_sysfs_dev);
    pr_info("WAYNE SYSFS 19.0: apertou pelo sysfs %d - KEY_A - Wayne Sysfs Paw\n", wayne_press_count);
    return count;
}

static struct kobj_attribute wayne_attr = __ATTR(press, 0664, wayne_show, wayne_store);

static int __init wayne_sysfs_init(void)
{
    int err;
    pr_info("WAYNE SYSFS 19.0: iniciando o sysfs\n");

    wayne_sysfs_dev = input_allocate_device();
    if (!wayne_sysfs_dev)
        return -ENOMEM;

    wayne_sysfs_dev->name = "Wayne Sysfs Paw";
    wayne_sysfs_dev->phys = "wayne/sysfs0";
    wayne_sysfs_dev->id.bustype = BUS_VIRTUAL;
    wayne_sysfs_dev->id.vendor = 0x1234;
    wayne_sysfs_dev->id.product = 0x0019;
    wayne_sysfs_dev->id.version = 0x1900;

    __set_bit(EV_KEY, wayne_sysfs_dev->evbit);
    __set_bit(EV_SYN, wayne_sysfs_dev->evbit);
    __set_bit(KEY_A, wayne_sysfs_dev->keybit);

    err = input_register_device(wayne_sysfs_dev);
    if (err) {
        input_free_device(wayne_sysfs_dev);
        pr_err("WAYNE SYSFS 19.0: falhou ao registrar\n");
        return err;
    }

    wayne_press_count = 0;
    wayne_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_kobj) {
        input_unregister_device(wayne_sysfs_dev);
        return -ENOMEM;
    }
    err = sysfs_create_file(wayne_kobj, &wayne_attr.attr);
    if (err) {
        kobject_put(wayne_kobj);
        input_unregister_device(wayne_sysfs_dev);
        return err;
    }

    pr_info("WAYNE SYSFS 19.0: sysfs criado em /sys/kernel/wayne/press - Wayne Sysfs Paw\n");
    return 0;
}

static void __exit wayne_sysfs_exit(void)
{
    sysfs_remove_file(wayne_kobj, &wayne_attr.attr);
    kobject_put(wayne_kobj);
    input_unregister_device(wayne_sysfs_dev);
    pr_info("WAYNE SYSFS 19.0: sysfs removido - apertou %d vezes\n", wayne_press_count);
}

module_init(wayne_sysfs_init);
module_exit(wayne_sysfs_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 19 - WAYNE SYSFS - aperta pelo sysfs");
