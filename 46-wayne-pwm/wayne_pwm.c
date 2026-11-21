#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/pwm.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_pwm_dev;
static struct kobject *wayne_pwm_kobj;
static int wayne_pwm_press;
static int wayne_pwm_duty;
static int wayne_pwm_period;
static int wayne_pwm_pulses;

static void wayne_pwm_do_pulse(void)
{
    wayne_pwm_duty += 10;
    if (wayne_pwm_duty > 100)
        wayne_pwm_duty = 0;
    wayne_pwm_pulses++;
    pr_info("WAYNE PWM 46.0: pulso %d - duty %d%% - period %d ns - PWM simulado\n", wayne_pwm_pulses, wayne_pwm_duty, wayne_pwm_period);
}

static ssize_t wayne_pwm_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "Wayne Pwm Paw - PWM 0 - press %d - duty %d%% - pulses %d\n", wayne_pwm_press, wayne_pwm_duty, wayne_pwm_pulses);
}

static ssize_t wayne_pwm_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_pwm_press++;
    wayne_pwm_do_pulse();
    input_report_key(wayne_pwm_dev, KEY_P, 1);
    input_sync(wayne_pwm_dev);
    input_report_key(wayne_pwm_dev, KEY_P, 0);
    input_sync(wayne_pwm_dev);
    pr_info("WAYNE PWM 46.0: aperto %d - duty %d%% - pulses %d - KEY_P - Wayne Pwm Paw - PWM 0\n", wayne_pwm_press, wayne_pwm_duty, wayne_pwm_pulses);
    return count;
}

static struct kobj_attribute wayne_pwm_attr = __ATTR(press, 0664, wayne_pwm_show, wayne_pwm_store);

static int __init wayne_pwm_init(void)
{
    int err;
    pr_info("WAYNE PWM 46.0: iniciando o pwm\n");

    wayne_pwm_dev = input_allocate_device();
    if (!wayne_pwm_dev)
        return -ENOMEM;

    wayne_pwm_dev->name = "Wayne Pwm Paw";
    wayne_pwm_dev->phys = "wayne/pwm0";
    wayne_pwm_dev->id.bustype = BUS_VIRTUAL;
    wayne_pwm_dev->id.vendor = 0x1234;
    wayne_pwm_dev->id.product = 0x0046;
    wayne_pwm_dev->id.version = 0x4600;

    __set_bit(EV_KEY, wayne_pwm_dev->evbit);
    __set_bit(EV_SYN, wayne_pwm_dev->evbit);
    __set_bit(KEY_P, wayne_pwm_dev->keybit);

    err = input_register_device(wayne_pwm_dev);
    if (err) {
        input_free_device(wayne_pwm_dev);
        return err;
    }

    wayne_pwm_press = 0;
    wayne_pwm_duty = 0;
    wayne_pwm_period = 1000000;
    wayne_pwm_pulses = 0;

    wayne_pwm_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_pwm_kobj) {
        input_unregister_device(wayne_pwm_dev);
        return -ENOMEM;
    }

    err = sysfs_create_file(wayne_pwm_kobj, &wayne_pwm_attr.attr);
    if (err) {
        kobject_put(wayne_pwm_kobj);
        input_unregister_device(wayne_pwm_dev);
        return err;
    }

    pr_info("WAYNE PWM 46.0: pwm criado - PWM 0 - period 1000000 ns - Wayne Pwm Paw - PWM simulado no WSL\n");
    return 0;
}

static void __exit wayne_pwm_exit(void)
{
    if (wayne_pwm_kobj) {
        sysfs_remove_file(wayne_pwm_kobj, &wayne_pwm_attr.attr);
        kobject_put(wayne_pwm_kobj);
    }
    input_unregister_device(wayne_pwm_dev);
    pr_info("WAYNE PWM 46.0: pwm removido - press %d - duty %d%% - pulses %d\n", wayne_pwm_press, wayne_pwm_duty, wayne_pwm_pulses);
}

module_init(wayne_pwm_init);
module_exit(wayne_pwm_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 46 - WAYNE PWM - pulsar");
