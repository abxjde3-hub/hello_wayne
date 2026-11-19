#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static struct input_dev *wayne_proc_dev;
static struct proc_dir_entry *wayne_proc_entry;
static int wayne_proc_count;

static int wayne_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Wayne Proc Paw - apertou %d vezes\n", wayne_proc_count);
    return 0;
}

static int wayne_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, wayne_proc_show, NULL);
}

static ssize_t wayne_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    wayne_proc_count++;
    input_report_key(wayne_proc_dev, KEY_A, 1);
    input_sync(wayne_proc_dev);
    input_report_key(wayne_proc_dev, KEY_A, 0);
    input_sync(wayne_proc_dev);
    pr_info("WAYNE PROC 21.0: apertou pelo proc %d - KEY_A - Wayne Proc Paw\n", wayne_proc_count);
    return count;
}

static const struct proc_ops wayne_proc_ops = {
    .proc_open = wayne_proc_open,
    .proc_read = seq_read,
    .proc_write = wayne_proc_write,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init wayne_proc_init(void)
{
    int err;
    pr_info("WAYNE PROC 21.0: iniciando o proc\n");

    wayne_proc_dev = input_allocate_device();
    if (!wayne_proc_dev)
        return -ENOMEM;

    wayne_proc_dev->name = "Wayne Proc Paw";
    wayne_proc_dev->phys = "wayne/proc0";
    wayne_proc_dev->id.bustype = BUS_VIRTUAL;
    wayne_proc_dev->id.vendor = 0x1234;
    wayne_proc_dev->id.product = 0x0021;
    wayne_proc_dev->id.version = 0x2100;

    __set_bit(EV_KEY, wayne_proc_dev->evbit);
    __set_bit(EV_SYN, wayne_proc_dev->evbit);
    __set_bit(KEY_A, wayne_proc_dev->keybit);

    err = input_register_device(wayne_proc_dev);
    if (err) {
        input_free_device(wayne_proc_dev);
        pr_err("WAYNE PROC 21.0: falhou ao registrar\n");
        return err;
    }

    wayne_proc_count = 0;
    wayne_proc_entry = proc_create("wayne", 0666, NULL, &wayne_proc_ops);
    if (!wayne_proc_entry) {
        input_unregister_device(wayne_proc_dev);
        return -ENOMEM;
    }

    pr_info("WAYNE PROC 21.0: proc criado em /proc/wayne - Wayne Proc Paw\n");
    return 0;
}

static void __exit wayne_proc_exit(void)
{
    proc_remove(wayne_proc_entry);
    input_unregister_device(wayne_proc_dev);
    pr_info("WAYNE PROC 21.0: proc removido - apertou %d vezes\n", wayne_proc_count);
}

module_init(wayne_proc_init);
module_exit(wayne_proc_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 21 - WAYNE PROC - aperta pelo proc");
