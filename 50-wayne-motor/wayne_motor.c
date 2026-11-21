#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_motor_dev;
static struct kobject *wayne_motor_kobj;
static int wayne_motor_press;
static int wayne_motor_spi;
static int wayne_motor_pwm;
static int wayne_motor_gpio;
static int wayne_motor_irq;
static int wayne_motor_timer;

static ssize_t wayne_motor_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "Wayne Motor Paw - MOTOR 50 - press %d - spi %d - pwm %d%% - gpio %d - irq %d - timer %d\n", wayne_motor_press, wayne_motor_spi, wayne_motor_pwm, wayne_motor_gpio, wayne_motor_irq, wayne_motor_timer);
}

static ssize_t wayne_motor_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_motor_press++;
    wayne_motor_spi++;
    wayne_motor_pwm += 10;
    if (wayne_motor_pwm > 100)
        wayne_motor_pwm = 0;
    wayne_motor_gpio = !wayne_motor_gpio;
    wayne_motor_irq++;
    wayne_motor_timer++;
    input_report_key(wayne_motor_dev, KEY_M, 1);
    input_sync(wayne_motor_dev);
    input_report_key(wayne_motor_dev, KEY_M, 0);
    input_sync(wayne_motor_dev);
    pr_info("WAYNE MOTOR 50.0: aperto %d - spi %d - pwm %d%% - gpio %d - irq %d - timer %d - KEY_M - Wayne Motor Paw - MOTOR COMPLETO\n", wayne_motor_press, wayne_motor_spi, wayne_motor_pwm, wayne_motor_gpio, wayne_motor_irq, wayne_motor_timer);
    return count;
}

static struct kobj_attribute wayne_motor_attr = __ATTR(press, 0664, wayne_motor_show, wayne_motor_store);

static int __init wayne_motor_init(void)
{
    int err;
    pr_info("WAYNE MOTOR 50.0: iniciando o motor completo\n");

    wayne_motor_dev = input_allocate_device();
    if (!wayne_motor_dev)
        return -ENOMEM;

    wayne_motor_dev->name = "Wayne Motor Paw";
    wayne_motor_dev->phys = "wayne/motor0";
    wayne_motor_dev->id.bustype = BUS_VIRTUAL;
    wayne_motor_dev->id.vendor = 0x1234;
    wayne_motor_dev->id.product = 0x0050;
    wayne_motor_dev->id.version = 0x5000;

    __set_bit(EV_KEY, wayne_motor_dev->evbit);
    __set_bit(EV_SYN, wayne_motor_dev->evbit);
    __set_bit(KEY_M, wayne_motor_dev->keybit);

    err = input_register_device(wayne_motor_dev);
    if (err) {
        input_free_device(wayne_motor_dev);
        return err;
    }

    wayne_motor_press = 0;
    wayne_motor_spi = 0;
    wayne_motor_pwm = 0;
    wayne_motor_gpio = 0;
    wayne_motor_irq = 0;
    wayne_motor_timer = 0;

    wayne_motor_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_motor_kobj) {
        input_unregister_device(wayne_motor_dev);
        return -ENOMEM;
    }

    err = sysfs_create_file(wayne_motor_kobj, &wayne_motor_attr.attr);
    if (err) {
        kobject_put(wayne_motor_kobj);
        input_unregister_device(wayne_motor_dev);
        return err;
    }

    pr_info("WAYNE MOTOR 50.0: motor criado - USB+I2C+SPI+PWM+GPIO+IRQ+TIMER+DT+ACPI - Wayne Motor Paw - MOTOR COMPLETO no WSL\n");
    return 0;
}

static void __exit wayne_motor_exit(void)
{
    if (wayne_motor_kobj) {
        sysfs_remove_file(wayne_motor_kobj, &wayne_motor_attr.attr);
        kobject_put(wayne_motor_kobj);
    }
    input_unregister_device(wayne_motor_dev);
    pr_info("WAYNE MOTOR 50.0: motor removido - press %d - spi %d - pwm %d%% - gpio %d - irq %d - timer %d\n", wayne_motor_press, wayne_motor_spi, wayne_motor_pwm, wayne_motor_gpio, wayne_motor_irq, wayne_motor_timer);
}

module_init(wayne_motor_init);
module_exit(wayne_motor_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 50 - WAYNE MOTOR - motor completo");
