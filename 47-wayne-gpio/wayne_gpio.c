#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_gpio_dev;
static struct kobject *wayne_gpio_kobj;
static int wayne_gpio_press;
static int wayne_gpio_pin;
static int wayne_gpio_value;
static int wayne_gpio_toggles;

static void wayne_gpio_do_toggle(void)
{
    wayne_gpio_value = !wayne_gpio_value;
    wayne_gpio_toggles++;
    pr_info("WAYNE GPIO 47.0: toggle %d - pino %d - valor %d - GPIO simulado\n", wayne_gpio_toggles, wayne_gpio_pin, wayne_gpio_value);
}

static ssize_t wayne_gpio_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "Wayne Gpio Paw - GPIO %d - press %d - valor %d - toggles %d\n", wayne_gpio_pin, wayne_gpio_press, wayne_gpio_value, wayne_gpio_toggles);
}

static ssize_t wayne_gpio_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_gpio_press++;
    wayne_gpio_do_toggle();
    input_report_key(wayne_gpio_dev, KEY_G, 1);
    input_sync(wayne_gpio_dev);
    input_report_key(wayne_gpio_dev, KEY_G, 0);
    input_sync(wayne_gpio_dev);
    pr_info("WAYNE GPIO 47.0: aperto %d - pino %d - valor %d - toggles %d - KEY_G - Wayne Gpio Paw - GPIO %d\n", wayne_gpio_press, wayne_gpio_pin, wayne_gpio_value, wayne_gpio_toggles, wayne_gpio_pin);
    return count;
}

static struct kobj_attribute wayne_gpio_attr = __ATTR(press, 0664, wayne_gpio_show, wayne_gpio_store);

static int __init wayne_gpio_init(void)
{
    int err;
    pr_info("WAYNE GPIO 47.0: iniciando o gpio\n");

    wayne_gpio_dev = input_allocate_device();
    if (!wayne_gpio_dev)
        return -ENOMEM;

    wayne_gpio_dev->name = "Wayne Gpio Paw";
    wayne_gpio_dev->phys = "wayne/gpio0";
    wayne_gpio_dev->id.bustype = BUS_VIRTUAL;
    wayne_gpio_dev->id.vendor = 0x1234;
    wayne_gpio_dev->id.product = 0x0047;
    wayne_gpio_dev->id.version = 0x4700;

    __set_bit(EV_KEY, wayne_gpio_dev->evbit);
    __set_bit(EV_SYN, wayne_gpio_dev->evbit);
    __set_bit(KEY_G, wayne_gpio_dev->keybit);

    err = input_register_device(wayne_gpio_dev);
    if (err) {
        input_free_device(wayne_gpio_dev);
        return err;
    }

    wayne_gpio_press = 0;
    wayne_gpio_pin = 47;
    wayne_gpio_value = 0;
    wayne_gpio_toggles = 0;

    wayne_gpio_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_gpio_kobj) {
        input_unregister_device(wayne_gpio_dev);
        return -ENOMEM;
    }

    err = sysfs_create_file(wayne_gpio_kobj, &wayne_gpio_attr.attr);
    if (err) {
        kobject_put(wayne_gpio_kobj);
        input_unregister_device(wayne_gpio_dev);
        return err;
    }

    pr_info("WAYNE GPIO 47.0: gpio criado - pino 47 - valor 0 - Wayne Gpio Paw - GPIO simulado no WSL\n");
    return 0;
}

static void __exit wayne_gpio_exit(void)
{
    if (wayne_gpio_kobj) {
        sysfs_remove_file(wayne_gpio_kobj, &wayne_gpio_attr.attr);
        kobject_put(wayne_gpio_kobj);
    }
    input_unregister_device(wayne_gpio_dev);
    pr_info("WAYNE GPIO 47.0: gpio removido - press %d - pino %d - valor %d - toggles %d\n", wayne_gpio_press, wayne_gpio_pin, wayne_gpio_value, wayne_gpio_toggles);
}

module_init(wayne_gpio_init);
module_exit(wayne_gpio_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 47 - WAYNE GPIO - pino");
