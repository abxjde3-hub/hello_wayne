#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/acpi.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_acpi_dev;
static struct platform_device *wayne_acpi_pdev;
static struct kobject *wayne_acpi_kobj;
static int wayne_acpi_press;
static int wayne_acpi_hid_num;
static int wayne_acpi_irq;

static ssize_t wayne_acpi_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "Wayne Acpi Paw - ACPI WAYN0001 - press %d - hid %d - irq %d\n", wayne_acpi_press, wayne_acpi_hid_num, wayne_acpi_irq);
}

static ssize_t wayne_acpi_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_acpi_press++;
    input_report_key(wayne_acpi_dev, KEY_A, 1);
    input_sync(wayne_acpi_dev);
    input_report_key(wayne_acpi_dev, KEY_A, 0);
    input_sync(wayne_acpi_dev);
    pr_info("WAYNE ACPI 44.0: aperto %d - hid %d - irq %d - KEY_A - Wayne Acpi Paw - ACPI WAYN0001\n", wayne_acpi_press, wayne_acpi_hid_num, wayne_acpi_irq);
    return count;
}

static struct kobj_attribute wayne_acpi_attr = __ATTR(press, 0664, wayne_acpi_show, wayne_acpi_store);

static int wayne_acpi_probe(struct platform_device *pdev)
{
    int err;
    pr_info("WAYNE ACPI 44.0: probe chamado - kernel achou pelo ACPI!\n");

    if (ACPI_COMPANION(&pdev->dev)) {
        pr_info("WAYNE ACPI 44.0: ACPI encontrado - HID WAYN0001\n");
        wayne_acpi_hid_num = 1;
        wayne_acpi_irq = 44;
    } else {
        wayne_acpi_hid_num = 1;
        wayne_acpi_irq = 44;
        pr_info("WAYNE ACPI 44.0: sem ACPI no WSL - usando padrao - HID WAYN0001 - irq 44 - simulado\n");
    }

    wayne_acpi_dev = input_allocate_device();
    if (!wayne_acpi_dev)
        return -ENOMEM;

    wayne_acpi_dev->name = "Wayne Acpi Paw";
    wayne_acpi_dev->phys = "wayne/acpi0";
    wayne_acpi_dev->id.bustype = BUS_HOST;
    wayne_acpi_dev->id.vendor = 0x1234;
    wayne_acpi_dev->id.product = 0x0044;
    wayne_acpi_dev->id.version = 0x4400;

    __set_bit(EV_KEY, wayne_acpi_dev->evbit);
    __set_bit(EV_SYN, wayne_acpi_dev->evbit);
    __set_bit(KEY_A, wayne_acpi_dev->keybit);

    err = input_register_device(wayne_acpi_dev);
    if (err) {
        input_free_device(wayne_acpi_dev);
        return err;
    }

    pr_info("WAYNE ACPI 44.0: probe OK - ACPI ligado - Wayne Acpi Paw\n");
    return 0;
}

static void wayne_acpi_remove(struct platform_device *pdev)
{
    input_unregister_device(wayne_acpi_dev);
    pr_info("WAYNE ACPI 44.0: remove - press %d - hid %d - irq %d\n", wayne_acpi_press, wayne_acpi_hid_num, wayne_acpi_irq);
}

static const struct acpi_device_id wayne_acpi_match[] = {
    { "WAYN0001", 0 },
    { },
};
MODULE_DEVICE_TABLE(acpi, wayne_acpi_match);

static struct platform_driver wayne_acpi_driver = {
    .probe = wayne_acpi_probe,
    .remove = wayne_acpi_remove,
    .driver = {
        .name = "wayne-acpi",
        .owner = THIS_MODULE,
        .acpi_match_table = wayne_acpi_match,
    },
};

static int __init wayne_acpi_init(void)
{
    int err;
    pr_info("WAYNE ACPI 44.0: iniciando o acpi\n");

    wayne_acpi_press = 0;
    wayne_acpi_hid_num = 1;
    wayne_acpi_irq = 44;

    wayne_acpi_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_acpi_kobj)
        return -ENOMEM;

    err = sysfs_create_file(wayne_acpi_kobj, &wayne_acpi_attr.attr);
    if (err) {
        kobject_put(wayne_acpi_kobj);
        return err;
    }

    wayne_acpi_pdev = platform_device_register_simple("wayne-acpi", -1, NULL, 0);
    if (IS_ERR(wayne_acpi_pdev)) {
        sysfs_remove_file(wayne_acpi_kobj, &wayne_acpi_attr.attr);
        kobject_put(wayne_acpi_kobj);
        return PTR_ERR(wayne_acpi_pdev);
    }

    err = platform_driver_register(&wayne_acpi_driver);
    if (err) {
        platform_device_unregister(wayne_acpi_pdev);
        sysfs_remove_file(wayne_acpi_kobj, &wayne_acpi_attr.attr);
        kobject_put(wayne_acpi_kobj);
        return err;
    }

    pr_info("WAYNE ACPI 44.0: acpi criado - HID WAYN0001 - Wayne Acpi Paw\n");
    return 0;
}

static void __exit wayne_acpi_exit(void)
{
    platform_driver_unregister(&wayne_acpi_driver);
    platform_device_unregister(wayne_acpi_pdev);
    if (wayne_acpi_kobj) {
        sysfs_remove_file(wayne_acpi_kobj, &wayne_acpi_attr.attr);
        kobject_put(wayne_acpi_kobj);
    }
    pr_info("WAYNE ACPI 44.0: acpi removido - press %d - hid %d - irq %d\n", wayne_acpi_press, wayne_acpi_hid_num, wayne_acpi_irq);
}

module_init(wayne_acpi_init);
module_exit(wayne_acpi_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 44 - WAYNE ACPI - mapa do PC");
