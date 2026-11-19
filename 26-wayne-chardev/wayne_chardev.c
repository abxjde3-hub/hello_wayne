#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

static struct input_dev *wayne_chr_dev;
static dev_t wayne_chr_num;
static struct cdev wayne_chr_cdev;
static struct class *wayne_chr_class;
static int wayne_chr_count;

static int wayne_chr_open(struct inode *inode, struct file *file)
{
    pr_info("WAYNE CHARDEV 26.0: abriu /dev/wayne\n");
    return 0;
}

static int wayne_chr_release(struct inode *inode, struct file *file)
{
    pr_info("WAYNE CHARDEV 26.0: fechou /dev/wayne\n");
    return 0;
}

static ssize_t wayne_chr_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char msg[64];
    int len;
    if (*ppos > 0)
        return 0;
    len = sprintf(msg, "Wayne Chardev Paw - apertou %d vezes\n", wayne_chr_count);
    if (copy_to_user(buf, msg, len))
        return -EFAULT;
    *ppos = len;
    return len;
}

static ssize_t wayne_chr_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    wayne_chr_count++;
    input_report_key(wayne_chr_dev, KEY_A, 1);
    input_sync(wayne_chr_dev);
    input_report_key(wayne_chr_dev, KEY_A, 0);
    input_sync(wayne_chr_dev);
    pr_info("WAYNE CHARDEV 26.0: escreveu no /dev/wayne %d - KEY_A - Wayne Chardev Paw\n", wayne_chr_count);
    return count;
}

static const struct file_operations wayne_chr_fops = {
    .owner = THIS_MODULE,
    .open = wayne_chr_open,
    .release = wayne_chr_release,
    .read = wayne_chr_read,
    .write = wayne_chr_write,
};

static int __init wayne_chr_init(void)
{
    int err;
    pr_info("WAYNE CHARDEV 26.0: iniciando o chardev\n");

    wayne_chr_dev = input_allocate_device();
    if (!wayne_chr_dev)
        return -ENOMEM;

    wayne_chr_dev->name = "Wayne Chardev Paw";
    wayne_chr_dev->phys = "wayne/chardev0";
    wayne_chr_dev->id.bustype = BUS_VIRTUAL;
    wayne_chr_dev->id.vendor = 0x1234;
    wayne_chr_dev->id.product = 0x0026;
    wayne_chr_dev->id.version = 0x2600;

    __set_bit(EV_KEY, wayne_chr_dev->evbit);
    __set_bit(EV_SYN, wayne_chr_dev->evbit);
    __set_bit(KEY_A, wayne_chr_dev->keybit);

    err = input_register_device(wayne_chr_dev);
    if (err) {
        input_free_device(wayne_chr_dev);
        return err;
    }

    wayne_chr_count = 0;
    err = alloc_chrdev_region(&wayne_chr_num, 0, 1, "wayne");
    if (err) {
        input_unregister_device(wayne_chr_dev);
        return err;
    }
    cdev_init(&wayne_chr_cdev, &wayne_chr_fops);
    err = cdev_add(&wayne_chr_cdev, wayne_chr_num, 1);
    if (err) {
        unregister_chrdev_region(wayne_chr_num, 1);
        input_unregister_device(wayne_chr_dev);
        return err;
    }
    wayne_chr_class = class_create("wayne");
    if (IS_ERR(wayne_chr_class)) {
        cdev_del(&wayne_chr_cdev);
        unregister_chrdev_region(wayne_chr_num, 1);
        input_unregister_device(wayne_chr_dev);
        return PTR_ERR(wayne_chr_class);
    }
    device_create(wayne_chr_class, NULL, wayne_chr_num, NULL, "wayne");

    pr_info("WAYNE CHARDEV 26.0: chardev criado em /dev/wayne - Wayne Chardev Paw\n");
    return 0;
}

static void __exit wayne_chr_exit(void)
{
    device_destroy(wayne_chr_class, wayne_chr_num);
    class_destroy(wayne_chr_class);
    cdev_del(&wayne_chr_cdev);
    unregister_chrdev_region(wayne_chr_num, 1);
    input_unregister_device(wayne_chr_dev);
    pr_info("WAYNE CHARDEV 26.0: chardev removido - apertou %d vezes\n", wayne_chr_count);
}

module_init(wayne_chr_init);
module_exit(wayne_chr_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 26 - WAYNE CHARDEV - /dev/wayne");
