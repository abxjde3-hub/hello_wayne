#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_pm_dev;
static struct kobject *wayne_pm_kobj;
static int wayne_pm_count;
static int wayne_pm_sleeping;

static void wayne_pm_fire(void)
{
    if (wayne_pm_sleeping) {
        pr_info("WAYNE PM 34.0: dormindo - nao aperta! Acorda primeiro\n");
        return;
    }
    wayne_pm_count++;
    input_report_key(wayne_pm_dev, KEY_S, 1);
    input_sync(wayne_pm_dev);
    input_report_key(wayne_pm_dev, KEY_S, 0);
    input_sync(wayne_pm_dev);
    pr_info("WAYNE PM 34.0: acordado - aperto %d - KEY_S - Wayne Pm Paw\n", wayne_pm_count);
}

static int wayne_pm_suspend(void)
{
    wayne_pm_sleeping = 1;
    pr_info("WAYNE PM 34.0: SUSPEND - indo dormir... Zzz - Wayne Pm Paw\n");
    return 0;
}

static int wayne_pm_resume(void)
{
    wayne_pm_sleeping = 0;
    pr_info("WAYNE PM 34.0: RESUME - acordou! Pronta de novo - Wayne Pm Paw\n");
    return 0;
}

static ssize_t wayne_pm_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    if (wayne_pm_sleeping)
        return sprintf(buf, "Wayne Pm Paw - DORMINDO - apertou %d vezes\n", wayne_pm_count);
    return sprintf(buf, "Wayne Pm Paw - ACORDADA - apertou %d vezes\n", wayne_pm_count);
}

static ssize_t wayne_pm_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_pm_fire();
    return count;
}

static ssize_t wayne_pm_sleep_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", wayne_pm_sleeping);
}

static ssize_t wayne_pm_sleep_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int v;
    if (kstrtoint(buf, 10, &v) == 0) {
        if (v)
            wayne_pm_suspend();
        else
            wayne_pm_resume();
    }
    return count;
}

static struct kobj_attribute wayne_pm_press_attr = __ATTR(press, 0664, wayne_pm_show, wayne_pm_store);
static struct kobj_attribute wayne_pm_sleep_attr = __ATTR(sleep, 0664, wayne_pm_sleep_show, wayne_pm_sleep_store);

static int __init wayne_pm_init(void)
{
    int err;
    pr_info("WAYNE PM 34.0: iniciando o pm\n");

    wayne_pm_dev = input_allocate_device();
    if (!wayne_pm_dev)
        return -ENOMEM;

    wayne_pm_dev->name = "Wayne Pm Paw";
    wayne_pm_dev->phys = "wayne/pm0";
    wayne_pm_dev->id.bustype = BUS_VIRTUAL;
    wayne_pm_dev->id.vendor = 0x1234;
    wayne_pm_dev->id.product = 0x0034;
    wayne_pm_dev->id.version = 0x3400;

    __set_bit(EV_KEY, wayne_pm_dev->evbit);
    __set_bit(EV_SYN, wayne_pm_dev->evbit);
    __set_bit(KEY_S, wayne_pm_dev->keybit);

    err = input_register_device(wayne_pm_dev);
    if (err) {
        input_free_device(wayne_pm_dev);
        return err;
    }

    wayne_pm_count = 0;
    wayne_pm_sleeping = 0;

    wayne_pm_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (wayne_pm_kobj) {
        sysfs_create_file(wayne_pm_kobj, &wayne_pm_press_attr.attr);
        sysfs_create_file(wayne_pm_kobj, &wayne_pm_sleep_attr.attr);
    }

    pr_info("WAYNE PM 34.0: pm criado - press em /sys/kernel/wayne/press - sleep em /sys/kernel/wayne/sleep - Wayne Pm Paw\n");
    return 0;
}

static void __exit wayne_pm_exit(void)
{
    if (wayne_pm_kobj) {
        sysfs_remove_file(wayne_pm_kobj, &wayne_pm_press_attr.attr);
        sysfs_remove_file(wayne_pm_kobj, &wayne_pm_sleep_attr.attr);
        kobject_put(wayne_pm_kobj);
    }
    input_unregister_device(wayne_pm_dev);
    pr_info("WAYNE PM 34.0: pm removido - apertou %d vezes - dormindo %d\n", wayne_pm_count, wayne_pm_sleeping);
}

module_init(wayne_pm_init);
module_exit(wayne_pm_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 34 - WAYNE PM - dormir e acordar");
