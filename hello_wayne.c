#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

static char wayne_msg[128] = "Hello from Wayne Kernel!";
static int read_count = 0;
static struct proc_dir_entry *wayne_entry;
static struct proc_dir_entry *count_entry;
static struct proc_dir_entry *reset_entry;

static ssize_t wayne_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char out[150];
    int len;
    if (*ppos > 0) return 0;
    read_count++;
    len = snprintf(out, sizeof(out), "%s\n", wayne_msg);
    return simple_read_from_buffer(buf, count, ppos, out, len);
}
static ssize_t wayne_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    if (count >= sizeof(wayne_msg)) return -EINVAL;
    if (copy_from_user(wayne_msg, buf, count)) return -EFAULT;
    wayne_msg[count] = '\0';
    if (count > 0 && wayne_msg[count-1] == '\n') wayne_msg[count-1] = '\0';
    return count;
}
static ssize_t count_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char out[32]; int len;
    if (*ppos > 0) return 0;
    len = snprintf(out, sizeof(out), "%d\n", read_count);
    return simple_read_from_buffer(buf, count, ppos, out, len);
}
static ssize_t reset_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    pr_info("wayne: RESET! era %d, agora 0\n", read_count);
    read_count = 0;
    return count;
}
static ssize_t reset_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char out[64]; int len;
    if (*ppos > 0) return 0;
    len = snprintf(out, sizeof(out), "Escreva p/ zerar. Atual: %d\n", read_count);
    return simple_read_from_buffer(buf, count, ppos, out, len);
}
static const struct proc_ops wayne_ops = { .proc_read = wayne_read, .proc_write = wayne_write, };
static const struct proc_ops count_ops = { .proc_read = count_read, };
static const struct proc_ops reset_ops = { .proc_read = reset_read, .proc_write = reset_write, };
static int __init wayne_init(void)
{
    wayne_entry = proc_create("wayne", 0666, NULL, &wayne_ops);
    count_entry = proc_create("wayne_count", 0444, NULL, &count_ops);
    reset_entry = proc_create("wayne_reset", 0666, NULL, &reset_ops);
    if (!wayne_entry || !count_entry || !reset_entry) return -ENOMEM;
    return 0;
}
static void __exit wayne_exit(void)
{
    proc_remove(wayne_entry); proc_remove(count_entry); proc_remove(reset_entry);
}
module_init(wayne_init);
module_exit(wayne_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne - abxjde3-hub");
MODULE_DESCRIPTION("Modulo Wayne 3 arquivos");
