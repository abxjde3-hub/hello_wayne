#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_u6_dev;
static struct platform_device *wayne_u6_pdev;
static struct timer_list wayne_u6_timer;
static struct kobject *wayne_u6_kobj;
static int wayne_u6_press;
static int wayne_u6_irq;
static int wayne_u6_gpio;
static int wayne_u6_sleeping;
static int wayne_u6_net;

static void wayne_u6_fire(int why)
{
    if (wayne_u6_sleeping) {
        pr_info("WAYNE ULTIMATE6 36.0: dormindo - nao aperta! why %d\n", why);
        return;
    }
    wayne_u6_press++;
    input_report_key(wayne_u6_dev, KEY_U, 1);
    input_sync(wayne_u6_dev);
    input_report_key(wayne_u6_dev, KEY_U, 0);
    input_sync(wayne_u6_dev);
    pr_info("WAYNE ULTIMATE6 36.0: aperto %d - why %d - gpio %d - irq %d - net %d - KEY_U\n", wayne_u6_press, why, wayne_u6_gpio, wayne_u6_irq, wayne_u6_net);
}

static void wayne_u6_timer_fn(struct timer_list *t)
{
    wayne_u6_irq++;
    wayne_u6_net++;
    pr_info("WAYNE ULTIMATE6 36.0: IRQ %d - NET TX %d - chamando...\n", wayne_u6_irq, wayne_u6_net);
    wayne_u6_fire(1);
    mod_timer(&wayne_u6_timer, jiffies + msecs_to_jiffies(4000));
}

static ssize_t wayne_u6_press_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "Wayne Ultimate6 Paw - press %d - irq %d - gpio %d - sleep %d - net %d\n", wayne_u6_press, wayne_u6_irq, wayne_u6_gpio, wayne_u6_sleeping, wayne_u6_net);
}

static ssize_t wayne_u6_press_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_u6_fire(2);
    return count;
}

static ssize_t wayne_u6_gpio_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", wayne_u6_gpio);
}

static ssize_t wayne_u6_gpio_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int v;
    if (kstrtoint(buf, 10, &v) == 0) {
        wayne_u6_gpio = (v != 0);
        wayne_u6_fire(3);
    }
    return count;
}

static ssize_t wayne_u6_sleep_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", wayne_u6_sleeping);
}

static ssize_t wayne_u6_sleep_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int v;
    if (kstrtoint(buf, 10, &v) == 0) {
        if (v) {
            wayne_u6_sleeping = 1;
            pr_info("WAYNE ULTIMATE6 36.0: SUSPEND - dormindo... Zzz\n");
        } else {
            wayne_u6_sleeping = 0;
            pr_info("WAYNE ULTIMATE6 36.0: RESUME - acordou!\n");
        }
    }
    return count;
}

static struct kobj_attribute wayne_u6_press_attr = __ATTR(press, 0664, wayne_u6_press_show, wayne_u6_press_store);
static struct kobj_attribute wayne_u6_gpio_attr = __ATTR(gpio, 0664, wayne_u6_gpio_show, wayne_u6_gpio_store);
static struct kobj_attribute wayne_u6_sleep_attr = __ATTR(sleep, 0664, wayne_u6_sleep_show, wayne_u6_sleep_store);

static int wayne_u6_probe(struct platform_device *pdev)
{
    int err;
    pr_info("WAYNE ULTIMATE6 36.0: probe chamado - kernel achou!\n");

    wayne_u6_dev = input_allocate_device();
    if (!wayne_u6_dev)
        return -ENOMEM;

    wayne_u6_dev->name = "Wayne Ultimate6 Paw";
    wayne_u6_dev->phys = "wayne/ultimate6";
    wayne_u6_dev->id.bustype = BUS_VIRTUAL;
    wayne_u6_dev->id.vendor = 0x1234;
    wayne_u6_dev->id.product = 0x0036;
    wayne_u6_dev->id.version = 0x3600;

    __set_bit(EV_KEY, wayne_u6_dev->evbit);
    __set_bit(EV_SYN, wayne_u6_dev->evbit);
    __set_bit(KEY_U, wayne_u6_dev->keybit);

    err = input_register_device(wayne_u6_dev);
    if (err) {
        input_free_device(wayne_u6_dev);
        return err;
    }

    wayne_u6_press = 0;
    wayne_u6_irq = 0;
    wayne_u6_gpio = 0;
    wayne_u6_sleeping = 0;
    wayne_u6_net = 0;

    wayne_u6_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (wayne_u6_kobj) {
        sysfs_create_file(wayne_u6_kobj, &wayne_u6_press_attr.attr);
        sysfs_create_file(wayne_u6_kobj, &wayne_u6_gpio_attr.attr);
        sysfs_create_file(wayne_u6_kobj, &wayne_u6_sleep_attr.attr);
    }

    timer_setup(&wayne_u6_timer, wayne_u6_timer_fn, 0);
    mod_timer(&wayne_u6_timer, jiffies + msecs_to_jiffies(4000));

    pr_info("WAYNE ULTIMATE6 36.0: probe OK - ultimate6 ligado\n");
    return 0;
}

static void wayne_u6_remove(struct platform_device *pdev)
{
    timer_delete_sync(&wayne_u6_timer);
    if (wayne_u6_kobj) {
        sysfs_remove_file(wayne_u6_kobj, &wayne_u6_press_attr.attr);
        sysfs_remove_file(wayne_u6_kobj, &wayne_u6_gpio_attr.attr);
        sysfs_remove_file(wayne_u6_kobj, &wayne_u6_sleep_attr.attr);
        kobject_put(wayne_u6_kobj);
    }
    input_unregister_device(wayne_u6_dev);
    pr_info("WAYNE ULTIMATE6 36.0: remove - press %d - irq %d - gpio %d - net %d\n", wayne_u6_press, wayne_u6_irq, wayne_u6_gpio, wayne_u6_net);
}

static struct platform_driver wayne_u6_driver = {
    .probe = wayne_u6_probe,
    .remove = wayne_u6_remove,
    .driver = { .name = "wayne-ultimate6", .owner = THIS_MODULE, },
};

static int __init wayne_u6_init(void)
{
    int err;
    pr_info("WAYNE ULTIMATE6 36.0: iniciando o ultimate6\n");
    wayne_u6_pdev = platform_device_register_simple("wayne-ultimate6", -1, NULL, 0);
    if (IS_ERR(wayne_u6_pdev))
        return PTR_ERR(wayne_u6_pdev);
    err = platform_driver_register(&wayne_u6_driver);
    if (err) {
        platform_device_unregister(wayne_u6_pdev);
        return err;
    }
    return 0;
}

static void __exit wayne_u6_exit(void)
{
    platform_driver_unregister(&wayne_u6_driver);
    platform_device_unregister(wayne_u6_pdev);
    pr_info("WAYNE ULTIMATE6 36.0: ultimate6 removido\n");
}

module_init(wayne_u6_init);
module_exit(wayne_u6_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 36 - WAYNE ULTIMATE6 - tudo junto dos 36");
