#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_ult_dev;
static struct timer_list wayne_ult_timer;
static struct kobject *wayne_ult_kobj;
static int wayne_ult_count;
static int wayne_ult_press;

static void wayne_ult_fn(struct timer_list *t)
{
    wayne_ult_count++;
    input_report_key(wayne_ult_dev, KEY_A, 1);
    input_sync(wayne_ult_dev);
    input_report_key(wayne_ult_dev, KEY_A, 0);
    input_sync(wayne_ult_dev);
    pr_info("WAYNE ULTIMATE3 20.0: batida %d - KEY_A - Wayne Ultimate3 Paw\n", wayne_ult_count);
    mod_timer(&wayne_ult_timer, jiffies + msecs_to_jiffies(2000));
}

static ssize_t wayne_ult_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "Wayne Ultimate3 Paw - batidas %d - apertos %d\n", wayne_ult_count, wayne_ult_press);
}

static ssize_t wayne_ult_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_ult_press++;
    input_report_key(wayne_ult_dev, KEY_W, 1);
    input_sync(wayne_ult_dev);
    input_report_key(wayne_ult_dev, KEY_W, 0);
    input_sync(wayne_ult_dev);
    input_report_rel(wayne_ult_dev, REL_X, 20);
    input_report_rel(wayne_ult_dev, REL_Y, 20);
    input_sync(wayne_ult_dev);
    input_report_abs(wayne_ult_dev, ABS_X, 5000);
    input_report_abs(wayne_ult_dev, ABS_Y, -5000);
    input_report_abs(wayne_ult_dev, ABS_Z, 800);
    input_sync(wayne_ult_dev);
    input_report_key(wayne_ult_dev, BTN_A, 1);
    input_sync(wayne_ult_dev);
    input_report_key(wayne_ult_dev, BTN_A, 0);
    input_sync(wayne_ult_dev);
    pr_info("WAYNE ULTIMATE3 20.0: aperto %d pelo sysfs - KEY_W + mouse + gamepad + sensor + BTN_A\n", wayne_ult_press);
    return count;
}

static struct kobj_attribute wayne_ult_attr = __ATTR(press, 0664, wayne_ult_show, wayne_ult_store);

static int __init wayne_ult_init(void)
{
    int err;
    pr_info("WAYNE ULTIMATE3 20.0: iniciando o ultimate3\n");

    wayne_ult_dev = input_allocate_device();
    if (!wayne_ult_dev)
        return -ENOMEM;

    wayne_ult_dev->name = "Wayne Ultimate3 Paw";
    wayne_ult_dev->phys = "wayne/ultimate3";
    wayne_ult_dev->id.bustype = BUS_VIRTUAL;
    wayne_ult_dev->id.vendor = 0x1234;
    wayne_ult_dev->id.product = 0x0020;
    wayne_ult_dev->id.version = 0x2000;

    __set_bit(EV_KEY, wayne_ult_dev->evbit);
    __set_bit(EV_ABS, wayne_ult_dev->evbit);
    __set_bit(EV_REL, wayne_ult_dev->evbit);
    __set_bit(EV_SYN, wayne_ult_dev->evbit);
    __set_bit(KEY_W, wayne_ult_dev->keybit);
    __set_bit(KEY_A, wayne_ult_dev->keybit);
    __set_bit(KEY_Y, wayne_ult_dev->keybit);
    __set_bit(KEY_N, wayne_ult_dev->keybit);
    __set_bit(KEY_E, wayne_ult_dev->keybit);
    __set_bit(BTN_LEFT, wayne_ult_dev->keybit);
    __set_bit(BTN_A, wayne_ult_dev->keybit);
    __set_bit(BTN_START, wayne_ult_dev->keybit);
    __set_bit(REL_X, wayne_ult_dev->relbit);
    __set_bit(REL_Y, wayne_ult_dev->relbit);

    input_set_abs_params(wayne_ult_dev, ABS_X, -32768, 32767, 0, 0);
    input_set_abs_params(wayne_ult_dev, ABS_Y, -32768, 32767, 0, 0);
    input_set_abs_params(wayne_ult_dev, ABS_Z, -1000, 1000, 0, 0);

    err = input_register_device(wayne_ult_dev);
    if (err) {
        input_free_device(wayne_ult_dev);
        pr_err("WAYNE ULTIMATE3 20.0: falhou ao registrar\n");
        return err;
    }

    wayne_ult_count = 0;
    wayne_ult_press = 0;
    wayne_ult_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_ult_kobj) {
        input_unregister_device(wayne_ult_dev);
        return -ENOMEM;
    }
    err = sysfs_create_file(wayne_ult_kobj, &wayne_ult_attr.attr);
    if (err) {
        kobject_put(wayne_ult_kobj);
        input_unregister_device(wayne_ult_dev);
        return err;
    }

    timer_setup(&wayne_ult_timer, wayne_ult_fn, 0);
    mod_timer(&wayne_ult_timer, jiffies + msecs_to_jiffies(2000));

    pr_info("WAYNE ULTIMATE3 20.0: ultimate3 ligado - timer + sysfs em /sys/kernel/wayne/press - Wayne Ultimate3 Paw\n");
    return 0;
}

static void __exit wayne_ult_exit(void)
{
    timer_delete_sync(&wayne_ult_timer);
    sysfs_remove_file(wayne_ult_kobj, &wayne_ult_attr.attr);
    kobject_put(wayne_ult_kobj);
    input_unregister_device(wayne_ult_dev);
    pr_info("WAYNE ULTIMATE3 20.0: ultimate3 removido - bateu %d - apertou %d\n", wayne_ult_count, wayne_ult_press);
}

module_init(wayne_ult_init);
module_exit(wayne_ult_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 20 - WAYNE ULTIMATE3 - tudo + timer + sysfs");
