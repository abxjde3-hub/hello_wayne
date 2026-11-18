#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne");
MODULE_DESCRIPTION("Projeto 4 - WAYNE CHAR DEVICE - /dev/wayne");

#define WAYNE_IOCTL_CLEAR _IO('w', 0)
#define WAYNE_IOCTL_GET_COUNT _IOR('w', 1, int)

static int major;
static struct class *wayne_class;
static struct device *wayne_device;
static char wayne_buffer[256];
static int wayne_len;
static int open_count;
static int total_writes;

static int wayne_open(struct inode *inode, struct file *file)
{
    open_count++;
    pr_info("WAYNE CHARDEV: aberto %d vezes\n", open_count);
    return 0;
}

static int wayne_release(struct inode *inode, struct file *file)
{
    pr_info("WAYNE CHARDEV: fechado\n");
    return 0;
}

static ssize_t wayne_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
    int to_copy;
    if (*ppos >= wayne_len)
        return 0;
    to_copy = min((int)len, wayne_len - (int)*ppos);
    if (copy_to_user(buf, wayne_buffer + *ppos, to_copy))
        return -EFAULT;
    *ppos += to_copy;
    return to_copy;
}

static ssize_t wayne_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
    int to_copy = min((int)len, 255);
    if (copy_from_user(wayne_buffer, buf, to_copy))
        return -EFAULT;
    wayne_buffer[to_copy] = '\0';
    wayne_len = to_copy;
    total_writes++;
    pr_info("WAYNE CHARDEV: recebeu %d bytes: %s\n", to_copy, wayne_buffer);
    return to_copy;
}

static long wayne_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int count = open_count;
    switch (cmd) {
    case WAYNE_IOCTL_CLEAR:
        wayne_buffer[0] = '\0';
        wayne_len = 0;
        pr_info("WAYNE CHARDEV: buffer limpo via ioctl\n");
        return 0;
    case WAYNE_IOCTL_GET_COUNT:
        if (copy_to_user((int __user *)arg, &count, sizeof(int)))
            return -EFAULT;
        return 0;
    default:
        return -EINVAL;
    }
}

static const struct file_operations wayne_fops = {
    .owner = THIS_MODULE,
    .open = wayne_open,
    .release = wayne_release,
    .read = wayne_read,
    .write = wayne_write,
    .unlocked_ioctl = wayne_ioctl,
};

static int __init wayne_chardev_init(void)
{
    major = register_chrdev(0, "wayne", &wayne_fops);
    if (major < 0) {
        pr_err("WAYNE CHARDEV: falhou registrar major\n");
        return major;
    }
    wayne_class = class_create("wayne_class");
    if (IS_ERR(wayne_class)) {
        unregister_chrdev(major, "wayne");
        return PTR_ERR(wayne_class);
    }
    wayne_device = device_create(wayne_class, NULL, MKDEV(major, 0), NULL, "wayne");
    if (IS_ERR(wayne_device)) {
        class_destroy(wayne_class);
        unregister_chrdev(major, "wayne");
        return PTR_ERR(wayne_device);
    }
    wayne_len = 0;
    open_count = 0;
    total_writes = 0;
    pr_info("WAYNE CHARDEV carregado - major %d - /dev/wayne criado\n", major);
    return 0;
}

static void __exit wayne_chardev_exit(void)
{
    device_destroy(wayne_class, MKDEV(major, 0));
    class_destroy(wayne_class);
    unregister_chrdev(major, "wayne");
    pr_info("WAYNE CHARDEV removido - aberturas: %d, escritas: %d\n", open_count, total_writes);
}

module_init(wayne_chardev_init);
module_exit(wayne_chardev_exit);
