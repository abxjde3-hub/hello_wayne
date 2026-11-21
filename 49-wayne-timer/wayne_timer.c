#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_timer_dev;
static struct kobject *wayne_timer_kobj;
static int wayne_timer_press;
static int wayne_timer_ticks;
static int wayne_timer_interval;

static void wayne_timer_do_tick(void)
{
    wayne_timer_ticks++;
    pr_info("WAYNE TIMER 49.0: tick %d - interval %d ms - TIMER simulado\n", wayne_timer_ticks, wayne_timer_interval);
}

static ssize_t wayne_timer_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "Wayne Timer Paw - TIMER - press %d - ticks %d - interval %d ms\n", wayne_timer_press, wayne_timer_ticks, wayne_timer_interval);
}

static ssize_t wayne_timer_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_timer_press++;
    wayne_timer_do_tick();
    input_report_key(wayne_timer_dev, KEY_T, 1);
    input_sync(wayne_timer_dev);
    input_report_key(wayne_timer_dev, KEY_T, 0);
    input_sync(wayne_timer_dev);
    pr_info("WAYNE TIMER 49.0: aperto %d - ticks %d - KEY_T - Wayne Timer Paw - TIMER\n", wayne_timer_press, wayne_timer_ticks);
    return count;
}

static struct kobj_attribute wayne_timer_attr = __ATTR(press, 0664, wayne_timer_show, wayne_timer_store);

static int __init wayne_timer_init(void)
{
    int err;
    pr_info("WAYNE TIMER 49.0: iniciando o timer\n");

    wayne_timer_dev = input_allocate_device();
    if (!wayne_timer_dev)
        return -ENOMEM;

    wayne_timer_dev->name = "Wayne Timer Paw";
    wayne_timer_dev->phys = "wayne/timer0";
    wayne_timer_dev->id.bustype = BUS_VIRTUAL;
    wayne_timer_dev->id.vendor = 0x1234;
    wayne_timer_dev->id.product = 0x0049;
    wayne_timer_dev->id.version = 0x4900;

    __set_bit(EV_KEY, wayne_timer_dev->evbit);
    __set_bit(EV_SYN, wayne_timer_dev->evbit);
    __set_bit(KEY_T, wayne_timer_dev->keybit);

    err = input_register_device(wayne_timer_dev);
    if (err) {
        input_free_device(wayne_timer_dev);
        return err;
    }

    wayne_timer_press = 0;
    wayne_timer_ticks = 0;
    wayne_timer_interval = 1000;

    wayne_timer_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_timer_kobj) {
        input_unregister_device(wayne_timer_dev);
        return -ENOMEM;
    }

    err = sysfs_create_file(wayne_timer_kobj, &wayne_timer_attr.attr);
    if (err) {
        kobject_put(wayne_timer_kobj);
        input_unregister_device(wayne_timer_dev);
        return err;
    }

    pr_info("WAYNE TIMER 49.0: timer criado - interval 1000 ms - Wayne Timer Paw - TIMER simulado no WSL\n");
    return 0;
}

static void __exit wayne_timer_exit(void)
{
    if (wayne_timer_kobj) {
        sysfs_remove_file(wayne_timer_kobj, &wayne_timer_attr.attr);
        kobject_put(wayne_timer_kobj);
    }
    input_unregister_device(wayne_timer_dev);
    pr_info("WAYNE TIMER 49.0: timer removido - press %d - ticks %d - interval %d ms\n", wayne_timer_press, wayne_timer_ticks, wayne_timer_interval);
}

module_init(wayne_timer_init);
module_exit(wayne_timer_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 49 - WAYNE TIMER - relogio");
