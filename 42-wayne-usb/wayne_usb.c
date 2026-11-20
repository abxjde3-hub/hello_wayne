#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_usb_dev;
static struct platform_device *wayne_usb_pdev;
static struct kobject *wayne_usb_kobj;
static int wayne_usb_press;
static int wayne_usb_plugs;

static ssize_t wayne_usb_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "Wayne Usb Paw - USB 1234:0042 simulado - press %d - plugs %d\n", wayne_usb_press, wayne_usb_plugs);
}

static ssize_t wayne_usb_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_usb_press++;
    input_report_key(wayne_usb_dev, KEY_U, 1);
    input_sync(wayne_usb_dev);
    input_report_key(wayne_usb_dev, KEY_U, 0);
    input_sync(wayne_usb_dev);
    pr_info("WAYNE USB 42.0: aperto %d - plugs %d - KEY_U - Wayne Usb Paw - USB 1234:0042 simulado\n", wayne_usb_press, wayne_usb_plugs);
    return count;
}

static struct kobj_attribute wayne_usb_attr = __ATTR(press, 0664, wayne_usb_show, wayne_usb_store);

static int wayne_usb_probe(struct platform_device *pdev)
{
    int err;
    pr_info("WAYNE USB 42.0: probe chamado - USB plugou! id 1234:0042 simulado\n");

    wayne_usb_dev = input_allocate_device();
    if (!wayne_usb_dev)
        return -ENOMEM;

    wayne_usb_dev->name = "Wayne Usb Paw";
    wayne_usb_dev->phys = "wayne/usb0";
    wayne_usb_dev->id.bustype = BUS_USB;
    wayne_usb_dev->id.vendor = 0x1234;
    wayne_usb_dev->id.product = 0x0042;
    wayne_usb_dev->id.version = 0x4200;

    __set_bit(EV_KEY, wayne_usb_dev->evbit);
    __set_bit(EV_SYN, wayne_usb_dev->evbit);
    __set_bit(KEY_U, wayne_usb_dev->keybit);

    err = input_register_device(wayne_usb_dev);
    if (err) {
        input_free_device(wayne_usb_dev);
        return err;
    }

    wayne_usb_plugs++;
    pr_info("WAYNE USB 42.0: probe OK - USB plugado - plugs %d - Wayne Usb Paw - simulado no WSL\n", wayne_usb_plugs);
    return 0;
}

static void wayne_usb_remove(struct platform_device *pdev)
{
    input_unregister_device(wayne_usb_dev);
    pr_info("WAYNE USB 42.0: disconnect - USB desplugou! - press %d - plugs %d\n", wayne_usb_press, wayne_usb_plugs);
}

static struct platform_driver wayne_usb_driver = {
    .probe = wayne_usb_probe,
    .remove = wayne_usb_remove,
    .driver = { .name = "wayne-usb", .owner = THIS_MODULE, },
};

static int __init wayne_usb_init(void)
{
    int err;
    pr_info("WAYNE USB 42.0: iniciando o usb\n");

    wayne_usb_press = 0;
    wayne_usb_plugs = 0;

    wayne_usb_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_usb_kobj)
        return -ENOMEM;

    err = sysfs_create_file(wayne_usb_kobj, &wayne_usb_attr.attr);
    if (err) {
        kobject_put(wayne_usb_kobj);
        return err;
    }

    wayne_usb_pdev = platform_device_register_simple("wayne-usb", -1, NULL, 0);
    if (IS_ERR(wayne_usb_pdev)) {
        sysfs_remove_file(wayne_usb_kobj, &wayne_usb_attr.attr);
        kobject_put(wayne_usb_kobj);
        return PTR_ERR(wayne_usb_pdev);
    }

    err = platform_driver_register(&wayne_usb_driver);
    if (err) {
        platform_device_unregister(wayne_usb_pdev);
        sysfs_remove_file(wayne_usb_kobj, &wayne_usb_attr.attr);
        kobject_put(wayne_usb_kobj);
        return err;
    }

    pr_info("WAYNE USB 42.0: usb criado - esperando plugar USB 1234:0042 simulado - Wayne Usb Paw\n");
    return 0;
}

static void __exit wayne_usb_exit(void)
{
    platform_driver_unregister(&wayne_usb_driver);
    platform_device_unregister(wayne_usb_pdev);
    if (wayne_usb_kobj) {
        sysfs_remove_file(wayne_usb_kobj, &wayne_usb_attr.attr);
        kobject_put(wayne_usb_kobj);
    }
    pr_info("WAYNE USB 42.0: usb removido - press %d - plugs %d\n", wayne_usb_press, wayne_usb_plugs);
}

module_init(wayne_usb_init);
module_exit(wayne_usb_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 42 - WAYNE USB - plugar e desplugar simulado");
