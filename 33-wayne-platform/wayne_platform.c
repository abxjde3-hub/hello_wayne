#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_pf_dev;
static struct platform_device *wayne_pf_pdev;
static int wayne_pf_count;

static ssize_t wayne_pf_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "Wayne Platform Paw - apertou %d vezes\n", wayne_pf_count);
}

static ssize_t wayne_pf_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_pf_count++;
    input_report_key(wayne_pf_dev, KEY_P, 1);
    input_sync(wayne_pf_dev);
    input_report_key(wayne_pf_dev, KEY_P, 0);
    input_sync(wayne_pf_dev);
    pr_info("WAYNE PLATFORM 33.0: probe apertou %d - KEY_P - Wayne Platform Paw\n", wayne_pf_count);
    return count;
}

static struct kobj_attribute wayne_pf_attr = __ATTR(press, 0664, wayne_pf_show, wayne_pf_store);
static struct kobject *wayne_pf_kobj;

static int wayne_pf_probe(struct platform_device *pdev)
{
    int err;
    pr_info("WAYNE PLATFORM 33.0: probe chamado - kernel achou o hardware!\n");

    wayne_pf_dev = input_allocate_device();
    if (!wayne_pf_dev)
        return -ENOMEM;

    wayne_pf_dev->name = "Wayne Platform Paw";
    wayne_pf_dev->phys = "wayne/platform0";
    wayne_pf_dev->id.bustype = BUS_VIRTUAL;
    wayne_pf_dev->id.vendor = 0x1234;
    wayne_pf_dev->id.product = 0x0033;
    wayne_pf_dev->id.version = 0x3300;

    __set_bit(EV_KEY, wayne_pf_dev->evbit);
    __set_bit(EV_SYN, wayne_pf_dev->evbit);
    __set_bit(KEY_P, wayne_pf_dev->keybit);

    err = input_register_device(wayne_pf_dev);
    if (err) {
        input_free_device(wayne_pf_dev);
        return err;
    }

    wayne_pf_count = 0;
    wayne_pf_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (wayne_pf_kobj)
        sysfs_create_file(wayne_pf_kobj, &wayne_pf_attr.attr);

    pr_info("WAYNE PLATFORM 33.0: probe OK - dispositivo criado - Wayne Platform Paw\n");
    return 0;
}

static void wayne_pf_remove(struct platform_device *pdev)
{
    if (wayne_pf_kobj) {
        sysfs_remove_file(wayne_pf_kobj, &wayne_pf_attr.attr);
        kobject_put(wayne_pf_kobj);
    }
    input_unregister_device(wayne_pf_dev);
    pr_info("WAYNE PLATFORM 33.0: remove chamado - apertou %d vezes\n", wayne_pf_count);
}

static struct platform_driver wayne_pf_driver = {
    .probe = wayne_pf_probe,
    .remove = wayne_pf_remove,
    .driver = {
        .name = "wayne-platform",
        .owner = THIS_MODULE,
    },
};

static int __init wayne_pf_init(void)
{
    int err;
    pr_info("WAYNE PLATFORM 33.0: iniciando o platform\n");

    wayne_pf_pdev = platform_device_register_simple("wayne-platform", -1, NULL, 0);
    if (IS_ERR(wayne_pf_pdev))
        return PTR_ERR(wayne_pf_pdev);

    err = platform_driver_register(&wayne_pf_driver);
    if (err) {
        platform_device_unregister(wayne_pf_pdev);
        return err;
    }

    pr_info("WAYNE PLATFORM 33.0: platform ligado - driver esperando hardware\n");
    return 0;
}

static void __exit wayne_pf_exit(void)
{
    platform_driver_unregister(&wayne_pf_driver);
    platform_device_unregister(wayne_pf_pdev);
    pr_info("WAYNE PLATFORM 33.0: platform removido\n");
}

module_init(wayne_pf_init);
module_exit(wayne_pf_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 33 - WAYNE PLATFORM - driver profissional");
