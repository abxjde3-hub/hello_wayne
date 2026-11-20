#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>

static struct input_dev *wayne_sp_dev;
static dev_t wayne_sp_num;
static struct cdev wayne_sp_cdev;
static struct class *wayne_sp_class;
static spinlock_t wayne_sp_lock;
static int wayne_sp_count;

static ssize_t wayne_sp_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char msg[64];
    int len;
    unsigned long flags;
    if (*ppos > 0)
        return 0;
    spin_lock_irqsave(&wayne_sp_lock, flags);
    len = sprintf(msg, "Wayne Spinlock Paw - apertou %d vezes\n", wayne_sp_count);
    spin_unlock_irqrestore(&wayne_sp_lock, flags);
    if (copy_to_user(buf, msg, len))
        return -EFAULT;
    *ppos = len;
    return len;
}

static ssize_t wayne_sp_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    unsigned long flags;
    spin_lock_irqsave(&wayne_sp_lock, flags);
    wayne_sp_count++;
    input_report_key(wayne_sp_dev, KEY_A, 1);
    input_sync(wayne_sp_dev);
    input_report_key(wayne_sp_dev, KEY_A, 0);
    input_sync(wayne_sp_dev);
    pr_info("WAYNE SPINLOCK 29.0: girou, travou, apertou %d, destravou - KEY_A - Wayne Spinlock Paw\n", wayne_sp_count);
    spin_unlock_irqrestore(&wayne_sp_lock, flags);
    return count;
}

static const struct file_operations wayne_sp_fops = {
    .owner = THIS_MODULE,
    .read = wayne_sp_read,
    .write = wayne_sp_write,
};

static int __init wayne_sp_init(void)
{
    int err;
    pr_info("WAYNE SPINLOCK 29.0: iniciando o spinlock\n");

    wayne_sp_dev = input_allocate_device();
    if (!wayne_sp_dev)
        return -ENOMEM;

    wayne_sp_dev->name = "Wayne Spinlock Paw";
    wayne_sp_dev->phys = "wayne/spinlock0";
    wayne_sp_dev->id.bustype = BUS_VIRTUAL;
    wayne_sp_dev->id.vendor = 0x1234;
    wayne_sp_dev->id.product = 0x0029;
    wayne_sp_dev->id.version = 0x2900;

    __set_bit(EV_KEY, wayne_sp_dev->evbit);
    __set_bit(EV_SYN, wayne_sp_dev->evbit);
    __set_bit(KEY_A, wayne_sp_dev->keybit);

    err = input_register_device(wayne_sp_dev);
    if (err) {
        input_free_device(wayne_sp_dev);
        return err;
    }

    wayne_sp_count = 0;
    spin_lock_init(&wayne_sp_lock);
    err = alloc_chrdev_region(&wayne_sp_num, 0, 1, "wayne_spinlock");
    if (err) {
        input_unregister_device(wayne_sp_dev);
        return err;
    }
    cdev_init(&wayne_sp_cdev, &wayne_sp_fops);
    err = cdev_add(&wayne_sp_cdev, wayne_sp_num, 1);
    if (err) {
        unregister_chrdev_region(wayne_sp_num, 1);
        input_unregister_device(wayne_sp_dev);
        return err;
    }
    wayne_sp_class = class_create("wayne");
    if (IS_ERR(wayne_sp_class)) {
        cdev_del(&wayne_sp_cdev);
        unregister_chrdev_region(wayne_sp_num, 1);
        input_unregister_device(wayne_sp_dev);
        return PTR_ERR(wayne_sp_class);
    }
    device_create(wayne_sp_class, NULL, wayne_sp_num, NULL, "wayne_spinlock");

    pr_info("WAYNE SPINLOCK 29.0: spinlock criado em /dev/wayne_spinlock - Wayne Spinlock Paw\n");
    return 0;
}

static void __exit wayne_sp_exit(void)
{
    device_destroy(wayne_sp_class, wayne_sp_num);
    class_destroy(wayne_sp_class);
    cdev_del(&wayne_sp_cdev);
    unregister_chrdev_region(wayne_sp_num, 1);
    input_unregister_device(wayne_sp_dev);
    pr_info("WAYNE SPINLOCK 29.0: spinlock removido - apertou %d vezes\n", wayne_sp_count);
}

module_init(wayne_sp_init);
module_exit(wayne_sp_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 29 - WAYNE SPINLOCK - gira e trava");
