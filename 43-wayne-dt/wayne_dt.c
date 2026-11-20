#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_dt_dev;
static struct platform_device *wayne_dt_pdev;
static struct kobject *wayne_dt_kobj;
static int wayne_dt_press;
static int wayne_dt_gpio;
static int wayne_dt_irq;

static ssize_t wayne_dt_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "Wayne Dt Paw - DT wayne,wayne-dt - press %d - gpio %d - irq %d\n", wayne_dt_press, wayne_dt_gpio, wayne_dt_irq);
}

static ssize_t wayne_dt_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_dt_press++;
    input_report_key(wayne_dt_dev, KEY_T, 1);
    input_sync(wayne_dt_dev);
    input_report_key(wayne_dt_dev, KEY_T, 0);
    input_sync(wayne_dt_dev);
    pr_info("WAYNE DT 43.0: aperto %d - gpio %d - irq %d - KEY_T - Wayne Dt Paw - DT wayne,wayne-dt\n", wayne_dt_press, wayne_dt_gpio, wayne_dt_irq);
    return count;
}

static struct kobj_attribute wayne_dt_attr = __ATTR(press, 0664, wayne_dt_show, wayne_dt_store);

static int wayne_dt_probe(struct platform_device *pdev)
{
    int err;
    struct device_node *np = pdev->dev.of_node;
    u32 val = 0;

    pr_info("WAYNE DT 43.0: probe chamado - kernel achou pelo Device Tree!\n");

    if (np) {
        of_property_read_u32(np, "wayne,gpio", &val);
        wayne_dt_gpio = val;
        of_property_read_u32(np, "wayne,irq", &val);
        wayne_dt_irq = val;
        pr_info("WAYNE DT 43.0: DT lido - gpio %d - irq %d - compatible wayne,wayne-dt\n", wayne_dt_gpio, wayne_dt_irq);
    } else {
        wayne_dt_gpio = 31;
        wayne_dt_irq = 43;
        pr_info("WAYNE DT 43.0: sem DT no WSL - usando padrao - gpio %d - irq %d - simulado\n", wayne_dt_gpio, wayne_dt_irq);
    }

    wayne_dt_dev = input_allocate_device();
    if (!wayne_dt_dev)
        return -ENOMEM;

    wayne_dt_dev->name = "Wayne Dt Paw";
    wayne_dt_dev->phys = "wayne/dt0";
    wayne_dt_dev->id.bustype = BUS_VIRTUAL;
    wayne_dt_dev->id.vendor = 0x1234;
    wayne_dt_dev->id.product = 0x0043;
    wayne_dt_dev->id.version = 0x4300;

    __set_bit(EV_KEY, wayne_dt_dev->evbit);
    __set_bit(EV_SYN, wayne_dt_dev->evbit);
    __set_bit(KEY_T, wayne_dt_dev->keybit);

    err = input_register_device(wayne_dt_dev);
    if (err) {
        input_free_device(wayne_dt_dev);
        return err;
    }

    pr_info("WAYNE DT 43.0: probe OK - DT ligado - Wayne Dt Paw\n");
    return 0;
}

static void wayne_dt_remove(struct platform_device *pdev)
{
    input_unregister_device(wayne_dt_dev);
    pr_info("WAYNE DT 43.0: remove - press %d - gpio %d - irq %d\n", wayne_dt_press, wayne_dt_gpio, wayne_dt_irq);
}

static const struct of_device_id wayne_dt_match[] = {
    { .compatible = "wayne,wayne-dt", },
    { },
};
MODULE_DEVICE_TABLE(of, wayne_dt_match);

static struct platform_driver wayne_dt_driver = {
    .probe = wayne_dt_probe,
    .remove = wayne_dt_remove,
    .driver = {
        .name = "wayne-dt",
        .owner = THIS_MODULE,
        .of_match_table = wayne_dt_match,
    },
};

static int __init wayne_dt_init(void)
{
    int err;
    pr_info("WAYNE DT 43.0: iniciando o dt\n");

    wayne_dt_press = 0;
    wayne_dt_gpio = 31;
    wayne_dt_irq = 43;

    wayne_dt_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_dt_kobj)
        return -ENOMEM;

    err = sysfs_create_file(wayne_dt_kobj, &wayne_dt_attr.attr);
    if (err) {
        kobject_put(wayne_dt_kobj);
        return err;
    }

    wayne_dt_pdev = platform_device_register_simple("wayne-dt", -1, NULL, 0);
    if (IS_ERR(wayne_dt_pdev)) {
        sysfs_remove_file(wayne_dt_kobj, &wayne_dt_attr.attr);
        kobject_put(wayne_dt_kobj);
        return PTR_ERR(wayne_dt_pdev);
    }

    err = platform_driver_register(&wayne_dt_driver);
    if (err) {
        platform_device_unregister(wayne_dt_pdev);
        sysfs_remove_file(wayne_dt_kobj, &wayne_dt_attr.attr);
        kobject_put(wayne_dt_kobj);
        return err;
    }

    pr_info("WAYNE DT 43.0: dt criado - compatible wayne,wayne-dt - Wayne Dt Paw\n");
    return 0;
}

static void __exit wayne_dt_exit(void)
{
    platform_driver_unregister(&wayne_dt_driver);
    platform_device_unregister(wayne_dt_pdev);
    if (wayne_dt_kobj) {
        sysfs_remove_file(wayne_dt_kobj, &wayne_dt_attr.attr);
        kobject_put(wayne_dt_kobj);
    }
    pr_info("WAYNE DT 43.0: dt removido - press %d - gpio %d - irq %d\n", wayne_dt_press, wayne_dt_gpio, wayne_dt_irq);
}

module_init(wayne_dt_init);
module_exit(wayne_dt_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 43 - WAYNE DT - device tree");
