#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/mm.h>
#include <linux/sched/signal.h>
#include <linux/utsname.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne");
MODULE_DESCRIPTION("Projeto 7 - WAYNE ULTIMATE - Driver completo");

#define WAYNE_IOCTL_CLEAR _IO('w', 0)
#define WAYNE_IOCTL_GET_COUNT _IOR('w', 1, int)

static struct timer_list wayne_timer;
static unsigned long start_jiffies;
static unsigned long timer_count;
static int major;
static struct class *wayne_class;
static struct device *wayne_device;
static struct kobject *wayne_kobj;

static char wayne_modo[32] = "normal";
static char wayne_mensagem[128] = "OLA DO ULTIMATE";
static int wayne_len;
static int wayne_contador;
static int open_count;
static int total_writes;

static void wayne_timer_callback(struct timer_list *t)
{
    timer_count++;
    mod_timer(&wayne_timer, jiffies + HZ);
}

// --- PROC ---
static int wayne_proc_show(struct seq_file *m, void *v)
{
    struct sysinfo si;
    int procs = 0;
    struct task_struct *p;
    si_meminfo(&si);
    for_each_process(p) procs++;
    seq_printf(m, "=== WAYNE ULTIMATE 7.0 ===\n");
    seq_printf(m, "Segundos rodando: %lu s\n", (jiffies - start_jiffies) / HZ);
    seq_printf(m, "Timer disparou: %lu vezes\n", timer_count);
    seq_printf(m, "Jiffies: %lu | HZ: %d\n", jiffies, HZ);
    seq_printf(m, "Processos: %d\n", procs);
    seq_printf(m, "Memoria total: %lu MB | Livre: %lu MB\n", (si.totalram * si.mem_unit) / 1024 / 1024, (si.freeram * si.mem_unit) / 1024 / 1024);
    seq_printf(m, "Modo: %s\n", wayne_modo);
    seq_printf(m, "Mensagem: %s\n", wayne_mensagem);
    seq_printf(m, "Contador sysfs+char: %d | Aberturas: %d | Escritas: %d\n", wayne_contador, open_count, total_writes);
    seq_printf(m, "Kernel: %s\n", init_utsname()->release);
    return 0;
}
static int wayne_proc_open(struct inode *inode, struct file *file) { return single_open(file, wayne_proc_show, NULL); }
static const struct proc_ops wayne_proc_ops = { .proc_open = wayne_proc_open, .proc_read = seq_read, .proc_lseek = seq_lseek, .proc_release = single_release };

// --- CHAR DEVICE ---
static int wayne_open(struct inode *inode, struct file *file) { open_count++; pr_info("WAYNE ULTIMATE: aberto %d vezes\n", open_count); return 0; }
static int wayne_release(struct inode *inode, struct file *file) { return 0; }
static ssize_t wayne_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
    int to_copy;
    if (*ppos >= wayne_len) return 0;
    to_copy = min((int)len, wayne_len - (int)*ppos);
    if (copy_to_user(buf, wayne_mensagem + *ppos, to_copy)) return -EFAULT;
    *ppos += to_copy;
    return to_copy;
}
static ssize_t wayne_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
    int to_copy = min((int)len, 127);
    if (copy_from_user(wayne_mensagem, buf, to_copy)) return -EFAULT;
    wayne_mensagem[to_copy] = '\0';
    wayne_len = to_copy;
    total_writes++; wayne_contador++;
    pr_info("WAYNE ULTIMATE: recebeu %d bytes: %s\n", to_copy, wayne_mensagem);
    return to_copy;
}
static long wayne_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int count = open_count;
    switch (cmd) {
    case WAYNE_IOCTL_CLEAR: wayne_mensagem[0] = '\0'; wayne_len = 0; pr_info("WAYNE ULTIMATE: buffer limpo via ioctl\n"); return 0;
    case WAYNE_IOCTL_GET_COUNT: if (copy_to_user((int __user *)arg, &count, sizeof(int))) return -EFAULT; return 0;
    default: return -EINVAL;
    }
}
static const struct file_operations wayne_fops = { .owner = THIS_MODULE, .open = wayne_open, .release = wayne_release, .read = wayne_read, .write = wayne_write, .unlocked_ioctl = wayne_ioctl };

// --- SYSFS ---
static ssize_t modo_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) { return sysfs_emit(buf, "%s\n", wayne_modo); }
static ssize_t modo_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    size_t len = min(count, sizeof(wayne_modo) - 1);
    memcpy(wayne_modo, buf, len); wayne_modo[len] = '\0';
    if (len > 0 && wayne_modo[len - 1] == '\n') wayne_modo[len - 1] = '\0';
    wayne_contador++; pr_info("WAYNE ULTIMATE: modo -> %s\n", wayne_modo); return count;
}
static ssize_t mensagem_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) { return sysfs_emit(buf, "%s\n", wayne_mensagem); }
static ssize_t mensagem_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    size_t len = min(count, sizeof(wayne_mensagem) - 1);
    memcpy(wayne_mensagem, buf, len); wayne_mensagem[len] = '\0';
    if (len > 0 && wayne_mensagem[len - 1] == '\n') wayne_mensagem[len - 1] = '\0';
    wayne_len = strlen(wayne_mensagem); wayne_contador++; total_writes++; return count;
}
static ssize_t contador_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) { return sysfs_emit(buf, "%d\n", wayne_contador); }
static ssize_t timer_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) { return sysfs_emit(buf, "%lu\n", timer_count); }

static struct kobj_attribute modo_attr = __ATTR(modo, 0664, modo_show, modo_store);
static struct kobj_attribute mensagem_attr = __ATTR(mensagem, 0664, mensagem_show, mensagem_store);
static struct kobj_attribute contador_attr = __ATTR(contador, 0444, contador_show, NULL);
static struct kobj_attribute timer_attr = __ATTR(timer, 0444, timer_show, NULL);
static struct attribute *wayne_attrs[] = { &modo_attr.attr, &mensagem_attr.attr, &contador_attr.attr, &timer_attr.attr, NULL };
static struct attribute_group wayne_group = { .attrs = wayne_attrs };

static int __init wayne_ultimate_init(void)
{
    int ret;
    start_jiffies = jiffies; timer_count = 0; wayne_contador = 0; open_count = 0; total_writes = 0; wayne_len = strlen(wayne_mensagem);
    proc_create("wayne_ultimate", 0, NULL, &wayne_proc_ops);
    timer_setup(&wayne_timer, wayne_timer_callback, 0);
    mod_timer(&wayne_timer, jiffies + HZ);
    major = register_chrdev(0, "wayne_final", &wayne_fops);
    if (major < 0) return major;
    wayne_class = class_create("wayne_final_class");
    if (IS_ERR(wayne_class)) { unregister_chrdev(major, "wayne_final"); return PTR_ERR(wayne_class); }
    wayne_device = device_create(wayne_class, NULL, MKDEV(major, 0), NULL, "wayne_final");
    if (IS_ERR(wayne_device)) { class_destroy(wayne_class); unregister_chrdev(major, "wayne_final"); return PTR_ERR(wayne_device); }
    wayne_kobj = kobject_create_and_add("wayne_final", kernel_kobj);
    if (!wayne_kobj) return -ENOMEM;
    ret = sysfs_create_group(wayne_kobj, &wayne_group);
    if (ret) return ret;
    pr_info("WAYNE ULTIMATE carregado - proc + dev + sysfs + timer\n");
    return 0;
}
static void __exit wayne_ultimate_exit(void)
{
    timer_delete_sync(&wayne_timer);
    sysfs_remove_group(wayne_kobj, &wayne_group);
    kobject_put(wayne_kobj);
    device_destroy(wayne_class, MKDEV(major, 0));
    class_destroy(wayne_class);
    unregister_chrdev(major, "wayne_final");
    remove_proc_entry("wayne_ultimate", NULL);
    pr_info("WAYNE ULTIMATE removido - timer %lu vezes, contador %d\n", timer_count, wayne_contador);
}
module_init(wayne_ultimate_init);
module_exit(wayne_ultimate_exit);
