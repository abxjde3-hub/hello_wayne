#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_NAME "wayne"

static char wayne_msg[128] = "Hello from Wayne Kernel!\n";
static struct proc_dir_entry *wayne_entry;

static ssize_t wayne_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    return simple_read_from_buffer(buf, count, ppos, wayne_msg, strlen(wayne_msg));
}

static ssize_t wayne_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    if (count >= sizeof(wayne_msg))
        return -EINVAL;

    if (copy_from_user(wayne_msg, buf, count))
        return -EFAULT;

    wayne_msg[count] = '\0';

    // tira o \n do echo
    if (count > 0 && wayne_msg[count-1] == '\n')
        wayne_msg[count-1] = '\0';

    pr_info("wayne: nova mensagem = %s\n", wayne_msg);
    return count;
}

static const struct proc_ops wayne_ops = {
    .proc_read = wayne_read,
    .proc_write = wayne_write,
};

static int __init wayne_init(void)
{
    wayne_entry = proc_create(PROC_NAME, 0666, NULL, &wayne_ops);
    if (!wayne_entry)
        return -ENOMEM;

    pr_info("wayne: modulo carregado! /proc/wayne pronto\n");
    return 0;
}

static void __exit wayne_exit(void)
{
    proc_remove(wayne_entry);
    pr_info("wayne: modulo removido!\n");
}

module_init(wayne_init);
module_exit(wayne_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne - abxjde3-hub");
MODULE_DESCRIPTION("Modulo /proc/wayne interativo - le e escreve");
