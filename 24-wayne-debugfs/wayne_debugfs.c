#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>

static struct input_dev *wayne_dbg_dev;
static struct dentry *wayne_dbg_dir;
static struct dentry *wayne_dbg_file;
static int wayne_dbg_count;

static int wayne_dbg_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Wayne Debugfs Paw - apertou %d vezes\n", wayne_dbg_count);
    return 0;
}

static int wayne_dbg_open(struct inode *inode, struct file *file)
{
    return single_open(file, wayne_dbg_show, NULL);
}

static ssize_t wayne_dbg_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    wayne_dbg_count++;
    input_report_key(wayne_dbg_dev, KEY_A, 1);
    input_sync(wayne_dbg_dev);
    input_report_key(wayne_dbg_dev, KEY_A, 0);
    input_sync(wayne_dbg_dev);
    pr_info("WAYNE DEBUGFS 24.0: apertou pelo debugfs %d - KEY_A - Wayne Debugfs Paw\n", wayne_dbg_count);
    return count;
}

static const struct file_operations wayne_dbg_fops = {
    .owner = THIS_MODULE,
    .open = wayne_dbg_open,
    .read = seq_read,
    .write = wayne_dbg_write,
    .llseek = seq_lseek,
    .release = single_release,
};

static int __init wayne_dbg_init(void)
{
    int err;
    pr_info("WAYNE DEBUGFS 24.0: iniciando o debugfs\n");

    wayne_dbg_dev = input_allocate_device();
    if (!wayne_dbg_dev)
        return -ENOMEM;

    wayne_dbg_dev->name = "Wayne Debugfs Paw";
    wayne_dbg_dev->phys = "wayne/debugfs0";
    wayne_dbg_dev->id.bustype = BUS_VIRTUAL;
    wayne_dbg_dev->id.vendor = 0x1234;
    wayne_dbg_dev->id.product = 0x0024;
    wayne_dbg_dev->id.version = 0x2400;

    __set_bit(EV_KEY, wayne_dbg_dev->evbit);
    __set_bit(EV_SYN, wayne_dbg_dev->evbit);
    __set_bit(KEY_A, wayne_dbg_dev->keybit);

    err = input_register_device(wayne_dbg_dev);
    if (err) {
        input_free_device(wayne_dbg_dev);
        pr_err("WAYNE DEBUGFS 24.0: falhou ao registrar\n");
        return err;
    }

    wayne_dbg_count = 0;
    wayne_dbg_dir = debugfs_create_dir("wayne", NULL);
    if (!wayne_dbg_dir) {
        input_unregister_device(wayne_dbg_dev);
        return -ENOMEM;
    }
    wayne_dbg_file = debugfs_create_file("press", 0666, wayne_dbg_dir, NULL, &wayne_dbg_fops);
    if (!wayne_dbg_file) {
        debugfs_remove_recursive(wayne_dbg_dir);
        input_unregister_device(wayne_dbg_dev);
        return -ENOMEM;
    }

    pr_info("WAYNE DEBUGFS 24.0: debugfs criado em /sys/kernel/debug/wayne/press - Wayne Debugfs Paw\n");
    return 0;
}

static void __exit wayne_dbg_exit(void)
{
    debugfs_remove_recursive(wayne_dbg_dir);
    input_unregister_device(wayne_dbg_dev);
    pr_info("WAYNE DEBUGFS 24.0: debugfs removido - apertou %d vezes\n", wayne_dbg_count);
}

module_init(wayne_dbg_init);
module_exit(wayne_dbg_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 24 - WAYNE DEBUGFS - aperta pelo debugfs");
