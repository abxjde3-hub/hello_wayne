#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_ck_dev;
static struct platform_device *wayne_ck_pdev;
static struct timer_list wayne_ck_timer;
static struct kobject *wayne_ck_kobj;
static int wayne_ck_press;
static int wayne_ck_irq;

static void wayne_ck_fire(void)
{
    wayne_ck_press++;
    input_report_key(wayne_ck_dev, KEY_C, 1);
    input_sync(wayne_ck_dev);
    input_report_key(wayne_ck_dev, KEY_C, 0);
    input_sync(wayne_ck_dev);
    pr_info("WAYNE CHECK 37.0: aperto %d - irq %d - KEY_C - Wayne Check Paw\n", wayne_ck_press, wayne_ck_irq);
}

static void wayne_ck_timer_fn(struct timer_list *t)
{
    wayne_ck_irq++;
    pr_info("WAYNE CHECK 37.0: IRQ %d - chamando...\n", wayne_ck_irq);
    wayne_ck_fire();
    mod_timer(&wayne_ck_timer, jiffies + msecs_to_jiffies(4000));
}

static ssize_t wayne_ck_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "Wayne Check Paw - press %d - irq %d\n", wayne_ck_press, wayne_ck_irq);
}

static ssize_t wayne_ck_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_ck_fire();
    return count;
}

static struct kobj_attribute wayne_ck_attr = __ATTR(press, 0664, wayne_ck_show, wayne_ck_store);

static int wayne_ck_probe(struct platform_device *pdev)
{
    int err;
    pr_info("WAYNE CHECK 37.0: probe chamado - kernel achou!\n");

    wayne_ck_dev = input_allocate_device();
    if (!wayne_ck_dev)
        return -ENOMEM;

    wayne_ck_dev->name = "Wayne Check Paw";
    wayne_ck_dev->phys = "wayne/check0";
    wayne_ck_dev->id.bustype = BUS_VIRTUAL;
    wayne_ck_dev->id.vendor = 0x1234;
    wayne_ck_dev->id.product = 0x0037;
    wayne_ck_dev->id.version = 0x3700;

    __set_bit(EV_KEY, wayne_ck_dev->evbit);
    __set_bit(EV_SYN, wayne_ck_dev->evbit);
    __set_bit(KEY_C, wayne_ck_dev->keybit);

    err = input_register_device(wayne_ck_dev);
    if (err) {
        input_free_device(wayne_ck_dev);
        return err;
    }

    wayne_ck_press = 0;
    wayne_ck_irq = 0;

    wayne_ck_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_ck_kobj) {
        input_unregister_device(wayne_ck_dev);
        return -ENOMEM;
    }

    err = sysfs_create_file(wayne_ck_kobj, &wayne_ck_attr.attr);
    if (err) {
        kobject_put(wayne_ck_kobj);
        input_unregister_device(wayne_ck_dev);
        return err;
    }

    timer_setup(&wayne_ck_timer, wayne_ck_timer_fn, 0);
    mod_timer(&wayne_ck_timer, jiffies + msecs_to_jiffies(4000));

    pr_info("WAYNE CHECK 37.0: probe OK - check ligado - sem warning!\n");
    return 0;
}

static void wayne_ck_remove(struct platform_device *pdev)
{
    timer_delete_sync(&wayne_ck_timer);
    if (wayne_ck_kobj) {
        sysfs_remove_file(wayne_ck_kobj, &wayne_ck_attr.attr);
        kobject_put(wayne_ck_kobj);
    }
    input_unregister_device(wayne_ck_dev);
    pr_info("WAYNE CHECK 37.0: remove - press %d - irq %d\n", wayne_ck_press, wayne_ck_irq);
}

static struct platform_driver wayne_ck_driver = {
    .probe = wayne_ck_probe,
    .remove = wayne_ck_remove,
    .driver = { .name = "wayne-check", .owner = THIS_MODULE, },
};

static int __init wayne_ck_init(void)
{
    int err;
    pr_info("WAYNE CHECK 37.0: iniciando o check\n");
    wayne_ck_pdev = platform_device_register_simple("wayne-check", -1, NULL, 0);
    if (IS_ERR(wayne_ck_pdev))
        return PTR_ERR(wayne_ck_pdev);
    err = platform_driver_register(&wayne_ck_driver);
    if (err) {
        platform_device_unregister(wayne_ck_pdev);
        return err;
    }
    return 0;
}

static void __exit wayne_ck_exit(void)
{
    platform_driver_unregister(&wayne_ck_driver);
    platform_device_unregister(wayne_ck_pdev);
    pr_info("WAYNE CHECK 37.0: check removido\n");
}

module_init(wayne_ck_init);
module_exit(wayne_ck_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 37 - WAYNE CHECK - codigo limpo");
