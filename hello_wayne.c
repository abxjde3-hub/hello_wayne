#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#define PROC_NAME "wayne"
#define BUFFER_SIZE 256
static char wayne_buffer[BUFFER_SIZE];
static struct proc_dir_entry *proc_entry;
static ssize_t wayne_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    int len;
    char tmp[BUFFER_SIZE];
    if (*ppos > 0) return 0;
    if (strlen(wayne_buffer) == 0)
        snprintf(tmp, BUFFER_SIZE, "Fala, Wayne! Escreva algo com: echo teste > /proc/wayne\n");
    else
        snprintf(tmp, BUFFER_SIZE, "Voce escreveu: %s\n", wayne_buffer);
    len = strlen(tmp);
    if (copy_to_user(ubuf, tmp, len)) return -EFAULT;
    *ppos = len;
    return len;
}
static ssize_t wayne_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    if (count >= BUFFER_SIZE) return -EINVAL;
    if (copy_from_user(wayne_buffer, ubuf, count)) return -EFAULT;
    wayne_buffer[count] = '\0';
    if (count > 0 && wayne_buffer[count-1] == '\n') wayne_buffer[count-1] = '\0';
    pr_info("wayne: voce escreveu: %s\n", wayne_buffer);
    return count;
}
static const struct proc_ops wayne_ops = { .proc_read = wayne_read, .proc_write = wayne_write, };
static int __init wayne_init(void)
{
    memset(wayne_buffer, 0, BUFFER_SIZE);
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &wayne_ops);
    if (!proc_entry) return -ENOMEM;
    pr_info("Hello, Wayne! /proc/wayne criado!\n");
    return 0;
}
static void __exit wayne_exit(void)
{
    proc_remove(proc_entry);
    pr_info("Tchau, Wayne!\n");
}
module_init(wayne_init);
module_exit(wayne_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne");
MODULE_DESCRIPTION("Modulo Wayne 2.0 - /proc interativo");
MODULE_VERSION("2.0");
