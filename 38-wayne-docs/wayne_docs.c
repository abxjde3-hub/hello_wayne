#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_dc_dev;
static struct kobject *wayne_dc_kobj;
static int wayne_dc_press;

static ssize_t wayne_dc_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "Wayne Docs Paw - Projeto 38 - Documentacao\nhello_wayne - Wayne Lab - 37 projetos - v4.0\nUse: cat /sys/kernel/wayne/docs para ver a doc\nAperto %d - KEY_D\n", wayne_dc_press);
}

static ssize_t wayne_dc_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_dc_press++;
    input_report_key(wayne_dc_dev, KEY_D, 1);
    input_sync(wayne_dc_dev);
    input_report_key(wayne_dc_dev, KEY_D, 0);
    input_sync(wayne_dc_dev);
    pr_info("WAYNE DOCS 38.0: aperto %d - KEY_D - Wayne Docs Paw - Documentacao\n", wayne_dc_press);
    return count;
}

static struct kobj_attribute wayne_dc_attr = __ATTR(press, 0664, wayne_dc_show, wayne_dc_store);
static struct kobj_attribute wayne_dc_docs_attr = __ATTR(docs, 0444, wayne_dc_show, NULL);

static int __init wayne_dc_init(void)
{
    int err;
    pr_info("WAYNE DOCS 38.0: iniciando o docs\n");

    wayne_dc_dev = input_allocate_device();
    if (!wayne_dc_dev)
        return -ENOMEM;

    wayne_dc_dev->name = "Wayne Docs Paw";
    wayne_dc_dev->phys = "wayne/docs0";
    wayne_dc_dev->id.bustype = BUS_VIRTUAL;
    wayne_dc_dev->id.vendor = 0x1234;
    wayne_dc_dev->id.product = 0x0038;
    wayne_dc_dev->id.version = 0x3800;

    __set_bit(EV_KEY, wayne_dc_dev->evbit);
    __set_bit(EV_SYN, wayne_dc_dev->evbit);
    __set_bit(KEY_D, wayne_dc_dev->keybit);

    err = input_register_device(wayne_dc_dev);
    if (err) {
        input_free_device(wayne_dc_dev);
        return err;
    }

    wayne_dc_press = 0;
    wayne_dc_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_dc_kobj) {
        input_unregister_device(wayne_dc_dev);
        return -ENOMEM;
    }

    err = sysfs_create_file(wayne_dc_kobj, &wayne_dc_attr.attr);
    if (err) {
        kobject_put(wayne_dc_kobj);
        input_unregister_device(wayne_dc_dev);
        return err;
    }

    err = sysfs_create_file(wayne_dc_kobj, &wayne_dc_docs_attr.attr);
    if (err) {
        sysfs_remove_file(wayne_dc_kobj, &wayne_dc_attr.attr);
        kobject_put(wayne_dc_kobj);
        input_unregister_device(wayne_dc_dev);
        return err;
    }

    pr_info("WAYNE DOCS 38.0: docs criado - docs em /sys/kernel/wayne/docs - Wayne Docs Paw\n");
    return 0;
}

static void __exit wayne_dc_exit(void)
{
    if (wayne_dc_kobj) {
        sysfs_remove_file(wayne_dc_kobj, &wayne_dc_attr.attr);
        sysfs_remove_file(wayne_dc_kobj, &wayne_dc_docs_attr.attr);
        kobject_put(wayne_dc_kobj);
    }
    input_unregister_device(wayne_dc_dev);
    pr_info("WAYNE DOCS 38.0: docs removido - apertou %d vezes\n", wayne_dc_press);
}

module_init(wayne_dc_init);
module_exit(wayne_dc_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 38 - WAYNE DOCS - documentacao");
