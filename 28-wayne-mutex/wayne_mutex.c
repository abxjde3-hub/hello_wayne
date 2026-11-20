#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/delay.h>

static struct input_dev *wayne_mx_dev;
static dev_t wayne_mx_num;
static struct cdev wayne_mx_cdev;
static struct class *wayne_mx_class;
static struct mutex wayne_mx_lock;
static int wayne_mx_count;

static ssize_t wayne_mx_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char msg[64];
    int len;
    if (*ppos > 0)
        return 0;
    mutex_lock(&wayne_mx_lock);
    len = sprintf(msg, "Wayne Mutex Paw - apertou %d vezes\n", wayne_mx_count);
    mutex_unlock(&wayne_mx_lock);
    if (copy_to_user(buf, msg, len))
        return -EFAULT;
    *ppos = len;
    return len;
}

static ssize_t wayne_mx_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    mutex_lock(&wayne_mx_lock);
    wayne_mx_count++;
    input_report_key(wayne_mx_dev, KEY_A, 1);
    input_sync(wayne_mx_dev);
    input_report_key(wayne_mx_dev, KEY_A, 0);
    input_sync(wayne_mx_dev);
    pr_info("WAYNE MUTEX 28.0: travou, apertou %d, destravou - KEY_A - Wayne Mutex Paw\n", wayne_mx_count);
    msleep(100);
    mutex_unlock(&wayne_mx_lock);
    return count;
}

static const struct file_operations wayne_mx_fops = {
    .owner = THIS_MODULE,
    .read = wayne_mx_read,
    .write = wayne_mx_write,
};

static int __init wayne_mx_init(void)
{
    int err;
    pr_info("WAYNE MUTEX 28.0: iniciando o mutex\n");

    wayne_mx_dev = input_allocate_device();
    if (!wayne_mx_dev)
        return -ENOMEM;

    wayne_mx_dev->name = "Wayne Mutex Paw";
    wayne_mx_dev->phys = "wayne/mutex0";
    wayne_mx_dev->id.bustype = BUS_VIRTUAL;
    wayne_mx_dev->id.vendor = 0x1234;
    wayne_mx_dev->id.product = 0x0028;
    wayne_mx_dev->id.version = 0x2800;

    __set_bit(EV_KEY, wayne_mx_dev->evbit);
    __set_bit(EV_SYN, wayne_mx_dev->evbit);
    __set_bit(KEY_A, wayne_mx_dev->keybit);

    err = input_register_device(wayne_mx_dev);
    if (err) {
        input_free_device(wayne_mx_dev);
        return err;
    }

    wayne_mx_count = 0;
    mutex_init(&wayne_mx_lock);
    err = alloc_chrdev_region(&wayne_mx_num, 0, 1, "wayne_mutex");
    if (err) {
        input_unregister_device(wayne_mx_dev);
        return err;
    }
    cdev_init(&wayne_mx_cdev, &wayne_mx_fops);
    err = cdev_add(&wayne_mx_cdev, wayne_mx_num, 1);
    if (err) {
        unregister_chrdev_region(wayne_mx_num, 1);
        input_unregister_device(wayne_mx_dev);
        return err;
    }
    wayne_mx_class = class_create("wayne");
    if (IS_ERR(wayne_mx_class)) {
        cdev_del(&wayne_mx_cdev);
        unregister_chrdev_region(wayne_mx_num, 1);
        input_unregister_device(wayne_mx_dev);
        return PTR_ERR(wayne_mx_class);
    }
    device_create(wayne_mx_class, NULL, wayne_mx_num, NULL, "wayne_mutex");

    pr_info("WAYNE MUTEX 28.0: mutex criado em /dev/wayne_mutex - Wayne Mutex Paw\n");
    return 0;
}

static void __exit wayne_mx_exit(void)
{
    device_destroy(wayne_mx_class, wayne_mx_num);
    class_destroy(wayne_mx_class);
    cdev_del(&wayne_mx_cdev);
    unregister_chrdev_region(wayne_mx_num, 1);
    input_unregister_device(wayne_mx_dev);
    pr_info("WAYNE MUTEX 28.0: mutex removido - apertou %d vezes\n", wayne_mx_count);
}

module_init(wayne_mx_init);
module_exit(wayne_mx_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 28 - WAYNE MUTEX - trava e destrava");
