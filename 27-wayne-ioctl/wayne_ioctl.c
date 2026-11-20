#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define WAYNE_IOC_MAGIC 'W'
#define WAYNE_IOC_PRESS _IO(WAYNE_IOC_MAGIC, 1)
#define WAYNE_IOC_RESET _IO(WAYNE_IOC_MAGIC, 2)
#define WAYNE_IOC_GET_COUNT _IOR(WAYNE_IOC_MAGIC, 3, int)

static struct input_dev *wayne_io_dev;
static dev_t wayne_io_num;
static struct cdev wayne_io_cdev;
static struct class *wayne_io_class;
static int wayne_io_count;

static long wayne_io_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int tmp;
    switch (cmd) {
    case WAYNE_IOC_PRESS:
        wayne_io_count++;
        input_report_key(wayne_io_dev, KEY_A, 1);
        input_sync(wayne_io_dev);
        input_report_key(wayne_io_dev, KEY_A, 0);
        input_sync(wayne_io_dev);
        pr_info("WAYNE IOCTL 27.0: ioctl PRESS %d - KEY_A - Wayne Ioctl Paw\n", wayne_io_count);
        return 0;
    case WAYNE_IOC_RESET:
        wayne_io_count = 0;
        pr_info("WAYNE IOCTL 27.0: ioctl RESET - contador zerado\n");
        return 0;
    case WAYNE_IOC_GET_COUNT:
        tmp = wayne_io_count;
        if (copy_to_user((int __user *)arg, &tmp, sizeof(tmp)))
            return -EFAULT;
        pr_info("WAYNE IOCTL 27.0: ioctl GET_COUNT = %d\n", tmp);
        return 0;
    default:
        return -ENOTTY;
    }
}

static int wayne_io_open(struct inode *inode, struct file *file)
{
    pr_info("WAYNE IOCTL 27.0: abriu /dev/wayne_ioctl\n");
    return 0;
}

static int wayne_io_release(struct inode *inode, struct file *file)
{
    pr_info("WAYNE IOCTL 27.0: fechou /dev/wayne_ioctl\n");
    return 0;
}

static const struct file_operations wayne_io_fops = {
    .owner = THIS_MODULE,
    .open = wayne_io_open,
    .release = wayne_io_release,
    .unlocked_ioctl = wayne_io_ioctl,
    .compat_ioctl = wayne_io_ioctl,
};

static int __init wayne_io_init(void)
{
    int err;
    pr_info("WAYNE IOCTL 27.0: iniciando o ioctl\n");

    wayne_io_dev = input_allocate_device();
    if (!wayne_io_dev)
        return -ENOMEM;

    wayne_io_dev->name = "Wayne Ioctl Paw";
    wayne_io_dev->phys = "wayne/ioctl0";
    wayne_io_dev->id.bustype = BUS_VIRTUAL;
    wayne_io_dev->id.vendor = 0x1234;
    wayne_io_dev->id.product = 0x0027;
    wayne_io_dev->id.version = 0x2700;

    __set_bit(EV_KEY, wayne_io_dev->evbit);
    __set_bit(EV_SYN, wayne_io_dev->evbit);
    __set_bit(KEY_A, wayne_io_dev->keybit);

    err = input_register_device(wayne_io_dev);
    if (err) {
        input_free_device(wayne_io_dev);
        return err;
    }

    wayne_io_count = 0;
    err = alloc_chrdev_region(&wayne_io_num, 0, 1, "wayne_ioctl");
    if (err) {
        input_unregister_device(wayne_io_dev);
        return err;
    }
    cdev_init(&wayne_io_cdev, &wayne_io_fops);
    err = cdev_add(&wayne_io_cdev, wayne_io_num, 1);
    if (err) {
        unregister_chrdev_region(wayne_io_num, 1);
        input_unregister_device(wayne_io_dev);
        return err;
    }
    wayne_io_class = class_create("wayne");
    if (IS_ERR(wayne_io_class)) {
        cdev_del(&wayne_io_cdev);
        unregister_chrdev_region(wayne_io_num, 1);
        input_unregister_device(wayne_io_dev);
        return PTR_ERR(wayne_io_class);
    }
    device_create(wayne_io_class, NULL, wayne_io_num, NULL, "wayne_ioctl");

    pr_info("WAYNE IOCTL 27.0: ioctl criado em /dev/wayne_ioctl - Wayne Ioctl Paw\n");
    return 0;
}

static void __exit wayne_io_exit(void)
{
    device_destroy(wayne_io_class, wayne_io_num);
    class_destroy(wayne_io_class);
    cdev_del(&wayne_io_cdev);
    unregister_chrdev_region(wayne_io_num, 1);
    input_unregister_device(wayne_io_dev);
    pr_info("WAYNE IOCTL 27.0: ioctl removido - apertou %d vezes\n", wayne_io_count);
}

module_init(wayne_io_init);
module_exit(wayne_io_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 27 - WAYNE IOCTL - comandos");
