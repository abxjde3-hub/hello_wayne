#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct input_dev *wayne_i2c_dev;
static struct kobject *wayne_i2c_kobj;
static int wayne_i2c_press;
static int wayne_i2c_temp;
static int wayne_i2c_reads;

static int wayne_i2c_read_temp(void)
{
    wayne_i2c_temp += 1;
    if (wayne_i2c_temp > 45)
        wayne_i2c_temp = 25;
    wayne_i2c_reads++;
    return wayne_i2c_temp;
}

static ssize_t wayne_i2c_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int t = wayne_i2c_read_temp();
    return sysfs_emit(buf, "Wayne I2C Paw - sensor 0x48 - temp %d C - press %d - reads %d\n", t, wayne_i2c_press, wayne_i2c_reads);
}

static ssize_t wayne_i2c_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int t = wayne_i2c_read_temp();
    wayne_i2c_press++;
    input_report_key(wayne_i2c_dev, KEY_I, 1);
    input_sync(wayne_i2c_dev);
    input_report_key(wayne_i2c_dev, KEY_I, 0);
    input_sync(wayne_i2c_dev);
    pr_info("WAYNE I2C 41.0: aperto %d - temp %d C - reads %d - KEY_I - Wayne I2C Paw - sensor 0x48\n", wayne_i2c_press, t, wayne_i2c_reads);
    return count;
}

static struct kobj_attribute wayne_i2c_attr = __ATTR(press, 0664, wayne_i2c_show, wayne_i2c_store);
static struct kobj_attribute wayne_i2c_temp_attr = __ATTR(temp, 0444, wayne_i2c_show, NULL);

static int __init wayne_i2c_init(void)
{
    int err;
    pr_info("WAYNE I2C 41.0: iniciando o i2c\n");

    wayne_i2c_dev = input_allocate_device();
    if (!wayne_i2c_dev)
        return -ENOMEM;

    wayne_i2c_dev->name = "Wayne I2C Paw";
    wayne_i2c_dev->phys = "wayne/i2c0";
    wayne_i2c_dev->id.bustype = BUS_I2C;
    wayne_i2c_dev->id.vendor = 0x1234;
    wayne_i2c_dev->id.product = 0x0041;
    wayne_i2c_dev->id.version = 0x4100;

    __set_bit(EV_KEY, wayne_i2c_dev->evbit);
    __set_bit(EV_SYN, wayne_i2c_dev->evbit);
    __set_bit(KEY_I, wayne_i2c_dev->keybit);

    err = input_register_device(wayne_i2c_dev);
    if (err) {
        input_free_device(wayne_i2c_dev);
        return err;
    }

    wayne_i2c_press = 0;
    wayne_i2c_temp = 25;
    wayne_i2c_reads = 0;

    wayne_i2c_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (!wayne_i2c_kobj) {
        input_unregister_device(wayne_i2c_dev);
        return -ENOMEM;
    }

    err = sysfs_create_file(wayne_i2c_kobj, &wayne_i2c_attr.attr);
    if (err) {
        kobject_put(wayne_i2c_kobj);
        input_unregister_device(wayne_i2c_dev);
        return err;
    }

    err = sysfs_create_file(wayne_i2c_kobj, &wayne_i2c_temp_attr.attr);
    if (err) {
        sysfs_remove_file(wayne_i2c_kobj, &wayne_i2c_attr.attr);
        kobject_put(wayne_i2c_kobj);
        input_unregister_device(wayne_i2c_dev);
        return err;
    }

    pr_info("WAYNE I2C 41.0: i2c criado - sensor 0x48 - temp 25 C - Wayne I2C Paw\n");
    return 0;
}

static void __exit wayne_i2c_exit(void)
{
    if (wayne_i2c_kobj) {
        sysfs_remove_file(wayne_i2c_kobj, &wayne_i2c_attr.attr);
        sysfs_remove_file(wayne_i2c_kobj, &wayne_i2c_temp_attr.attr);
        kobject_put(wayne_i2c_kobj);
    }
    input_unregister_device(wayne_i2c_dev);
    pr_info("WAYNE I2C 41.0: i2c removido - press %d - reads %d - temp %d C\n", wayne_i2c_press, wayne_i2c_reads, wayne_i2c_temp);
}

module_init(wayne_i2c_init);
module_exit(wayne_i2c_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 41 - WAYNE I2C - sensor");
