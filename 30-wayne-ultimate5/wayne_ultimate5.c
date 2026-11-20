#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>

#define WAYNE_IOC_MAGIC 'W'
#define WAYNE_IOC_PRESS _IO(WAYNE_IOC_MAGIC, 1)
#define WAYNE_IOC_RESET _IO(WAYNE_IOC_MAGIC, 2)
#define WAYNE_IOC_GET_COUNT _IOR(WAYNE_IOC_MAGIC, 3, int)

static struct input_dev *wayne_u5_dev;
static struct timer_list wayne_u5_timer;
static struct kobject *wayne_u5_kobj;
static struct proc_dir_entry *wayne_u5_proc;
static struct dentry *wayne_u5_dbg_dir;
static struct dentry *wayne_u5_dbg_file;
static struct work_struct wayne_u5_work;
static struct task_struct *wayne_u5_task;
static dev_t wayne_u5_num;
static struct cdev wayne_u5_cdev;
static struct class *wayne_u5_class;
static struct mutex wayne_u5_mutex;
static spinlock_t wayne_u5_spin;
static int wayne_u5_timer_count;
static int wayne_u5_press;
static int wayne_u5_work_count;
static int wayne_u5_kt_count;

static void wayne_u5_fire_locked(void)
{
    wayne_u5_press++;
    input_report_key(wayne_u5_dev, KEY_W, 1);
    input_sync(wayne_u5_dev);
    input_report_key(wayne_u5_dev, KEY_W, 0);
    input_sync(wayne_u5_dev);
    input_report_rel(wayne_u5_dev, REL_X, 20);
    input_report_rel(wayne_u5_dev, REL_Y, 20);
    input_sync(wayne_u5_dev);
    input_report_abs(wayne_u5_dev, ABS_X, 5000);
    input_report_abs(wayne_u5_dev, ABS_Y, -5000);
    input_report_abs(wayne_u5_dev, ABS_Z, 800);
    input_sync(wayne_u5_dev);
    input_report_key(wayne_u5_dev, BTN_A, 1);
    input_sync(wayne_u5_dev);
    input_report_key(wayne_u5_dev, BTN_A, 0);
    input_sync(wayne_u5_dev);
    pr_info("WAYNE ULTIMATE5 30.0: aperto %d - KEY_W + mouse + gamepad + sensor + BTN_A\n", wayne_u5_press);
}

static void wayne_u5_fire(void)
{
    unsigned long flags;
    mutex_lock(&wayne_u5_mutex);
    spin_lock_irqsave(&wayne_u5_spin, flags);
    wayne_u5_fire_locked();
    spin_unlock_irqrestore(&wayne_u5_spin, flags);
    mutex_unlock(&wayne_u5_mutex);
}

static void wayne_u5_timer_fn(struct timer_list *t)
{
    unsigned long flags;
    spin_lock_irqsave(&wayne_u5_spin, flags);
    wayne_u5_timer_count++;
    input_report_key(wayne_u5_dev, KEY_A, 1);
    input_sync(wayne_u5_dev);
    input_report_key(wayne_u5_dev, KEY_A, 0);
    input_sync(wayne_u5_dev);
    pr_info("WAYNE ULTIMATE5 30.0: batida timer %d - KEY_A\n", wayne_u5_timer_count);
    spin_unlock_irqrestore(&wayne_u5_spin, flags);
    mod_timer(&wayne_u5_timer, jiffies + msecs_to_jiffies(2000));
}

static void wayne_u5_work_fn(struct work_struct *work)
{
    wayne_u5_work_count++;
    input_report_key(wayne_u5_dev, KEY_A, 1);
    input_sync(wayne_u5_dev);
    input_report_key(wayne_u5_dev, KEY_A, 0);
    input_sync(wayne_u5_dev);
    pr_info("WAYNE ULTIMATE5 30.0: trabalho %d em segundo plano - KEY_A\n", wayne_u5_work_count);
}

static int wayne_u5_kt_fn(void *data)
{
    while (!kthread_should_stop()) {
        wayne_u5_kt_count++;
        input_report_key(wayne_u5_dev, KEY_A, 1);
        input_sync(wayne_u5_dev);
        input_report_key(wayne_u5_dev, KEY_A, 0);
        input_sync(wayne_u5_dev);
        pr_info("WAYNE ULTIMATE5 30.0: volta kthread %d - KEY_A\n", wayne_u5_kt_count);
        msleep(5000);
    }
    return 0;
}

static ssize_t wayne_u5_sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "Wayne Ultimate5 Paw - timer %d - apertos %d - work %d - kthread %d\n", wayne_u5_timer_count, wayne_u5_press, wayne_u5_work_count, wayne_u5_kt_count);
}

static ssize_t wayne_u5_sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_u5_fire();
    return count;
}

static struct kobj_attribute wayne_u5_attr = __ATTR(press, 0664, wayne_u5_sysfs_show, wayne_u5_sysfs_store);

static int wayne_u5_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Wayne Ultimate5 Paw - timer %d - apertos %d - work %d - kthread %d\n", wayne_u5_timer_count, wayne_u5_press, wayne_u5_work_count, wayne_u5_kt_count);
    return 0;
}

static int wayne_u5_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, wayne_u5_proc_show, NULL);
}

static ssize_t wayne_u5_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    wayne_u5_fire();
    return count;
}

static const struct proc_ops wayne_u5_proc_ops = {
    .proc_open = wayne_u5_proc_open,
    .proc_read = seq_read,
    .proc_write = wayne_u5_proc_write,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int wayne_u5_dbg_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Wayne Ultimate5 Paw - timer %d - apertos %d - work %d - kthread %d\n", wayne_u5_timer_count, wayne_u5_press, wayne_u5_work_count, wayne_u5_kt_count);
    return 0;
}

static int wayne_u5_dbg_open(struct inode *inode, struct file *file)
{
    return single_open(file, wayne_u5_dbg_show, NULL);
}

static ssize_t wayne_u5_dbg_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    wayne_u5_fire();
    return count;
}

static const struct file_operations wayne_u5_dbg_fops = {
    .owner = THIS_MODULE,
    .open = wayne_u5_dbg_open,
    .read = seq_read,
    .write = wayne_u5_dbg_write,
    .llseek = seq_lseek,
    .release = single_release,
};

static ssize_t wayne_u5_chr_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char msg[128];
    int len;
    if (*ppos > 0)
        return 0;
    mutex_lock(&wayne_u5_mutex);
    len = sprintf(msg, "Wayne Ultimate5 Paw - timer %d - apertos %d - work %d - kthread %d\n", wayne_u5_timer_count, wayne_u5_press, wayne_u5_work_count, wayne_u5_kt_count);
    mutex_unlock(&wayne_u5_mutex);
    if (copy_to_user(buf, msg, len))
        return -EFAULT;
    *ppos = len;
    return len;
}

static ssize_t wayne_u5_chr_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    wayne_u5_fire();
    return count;
}

static long wayne_u5_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int tmp;
    switch (cmd) {
    case WAYNE_IOC_PRESS:
        wayne_u5_fire();
        return 0;
    case WAYNE_IOC_RESET:
        mutex_lock(&wayne_u5_mutex);
        wayne_u5_press = 0;
        mutex_unlock(&wayne_u5_mutex);
        pr_info("WAYNE ULTIMATE5 30.0: ioctl RESET - contador zerado\n");
        return 0;
    case WAYNE_IOC_GET_COUNT:
        tmp = wayne_u5_press;
        if (copy_to_user((int __user *)arg, &tmp, sizeof(tmp)))
            return -EFAULT;
        pr_info("WAYNE ULTIMATE5 30.0: ioctl GET_COUNT = %d\n", tmp);
        return 0;
    default:
        return -ENOTTY;
    }
}

static const struct file_operations wayne_u5_chr_fops = {
    .owner = THIS_MODULE,
    .read = wayne_u5_chr_read,
    .write = wayne_u5_chr_write,
    .unlocked_ioctl = wayne_u5_ioctl,
    .compat_ioctl = wayne_u5_ioctl,
};

static int __init wayne_u5_init(void)
{
    int err;
    pr_info("WAYNE ULTIMATE5 30.0: iniciando o ultimate5\n");

    wayne_u5_dev = input_allocate_device();
    if (!wayne_u5_dev)
        return -ENOMEM;

    wayne_u5_dev->name = "Wayne Ultimate5 Paw";
    wayne_u5_dev->phys = "wayne/ultimate5";
    wayne_u5_dev->id.bustype = BUS_VIRTUAL;
    wayne_u5_dev->id.vendor = 0x1234;
    wayne_u5_dev->id.product = 0x0030;
    wayne_u5_dev->id.version = 0x3000;

    __set_bit(EV_KEY, wayne_u5_dev->evbit);
    __set_bit(EV_ABS, wayne_u5_dev->evbit);
    __set_bit(EV_REL, wayne_u5_dev->evbit);
    __set_bit(EV_SYN, wayne_u5_dev->evbit);
    __set_bit(KEY_W, wayne_u5_dev->keybit);
    __set_bit(KEY_A, wayne_u5_dev->keybit);
    __set_bit(KEY_Y, wayne_u5_dev->keybit);
    __set_bit(KEY_N, wayne_u5_dev->keybit);
    __set_bit(KEY_E, wayne_u5_dev->keybit);
    __set_bit(BTN_LEFT, wayne_u5_dev->keybit);
    __set_bit(BTN_A, wayne_u5_dev->keybit);
    __set_bit(BTN_START, wayne_u5_dev->keybit);
    __set_bit(REL_X, wayne_u5_dev->relbit);
    __set_bit(REL_Y, wayne_u5_dev->relbit);

    input_set_abs_params(wayne_u5_dev, ABS_X, -32768, 32767, 0, 0);
    input_set_abs_params(wayne_u5_dev, ABS_Y, -32768, 32767, 0, 0);
    input_set_abs_params(wayne_u5_dev, ABS_Z, -1000, 1000, 0, 0);

    err = input_register_device(wayne_u5_dev);
    if (err) {
        input_free_device(wayne_u5_dev);
        return err;
    }

    wayne_u5_timer_count = 0;
    wayne_u5_press = 0;
    wayne_u5_work_count = 0;
    wayne_u5_kt_count = 0;
    mutex_init(&wayne_u5_mutex);
    spin_lock_init(&wayne_u5_spin);

    wayne_u5_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (wayne_u5_kobj)
        sysfs_create_file(wayne_u5_kobj, &wayne_u5_attr.attr);

    wayne_u5_proc = proc_create("wayne", 0666, NULL, &wayne_u5_proc_ops);

    wayne_u5_dbg_dir = debugfs_create_dir("wayne", NULL);
    if (wayne_u5_dbg_dir)
        wayne_u5_dbg_file = debugfs_create_file("press", 0666, wayne_u5_dbg_dir, NULL, &wayne_u5_dbg_fops);

    err = alloc_chrdev_region(&wayne_u5_num, 0, 1, "wayne_ultimate5");
    if (err == 0) {
        cdev_init(&wayne_u5_cdev, &wayne_u5_chr_fops);
        if (cdev_add(&wayne_u5_cdev, wayne_u5_num, 1) == 0) {
            wayne_u5_class = class_create("wayne");
            if (!IS_ERR(wayne_u5_class))
                device_create(wayne_u5_class, NULL, wayne_u5_num, NULL, "wayne_ultimate5");
        }
    }

    timer_setup(&wayne_u5_timer, wayne_u5_timer_fn, 0);
    mod_timer(&wayne_u5_timer, jiffies + msecs_to_jiffies(2000));

    INIT_WORK(&wayne_u5_work, wayne_u5_work_fn);
    schedule_work(&wayne_u5_work);

    wayne_u5_task = kthread_run(wayne_u5_kt_fn, NULL, "wayne_ultimate5");

    pr_info("WAYNE ULTIMATE5 30.0: ultimate5 ligado - timer + sysfs + proc + debugfs + workqueue + kthread + chardev + ioctl + mutex + spinlock - Wayne Ultimate5 Paw\n");
    return 0;
}

static void __exit wayne_u5_exit(void)
{
    if (wayne_u5_task)
        kthread_stop(wayne_u5_task);
    timer_delete_sync(&wayne_u5_timer);
    flush_work(&wayne_u5_work);
    cancel_work_sync(&wayne_u5_work);
    if (wayne_u5_class) {
        device_destroy(wayne_u5_class, wayne_u5_num);
        class_destroy(wayne_u5_class);
    }
    cdev_del(&wayne_u5_cdev);
    unregister_chrdev_region(wayne_u5_num, 1);
    if (wayne_u5_dbg_dir)
        debugfs_remove_recursive(wayne_u5_dbg_dir);
    if (wayne_u5_proc)
        proc_remove(wayne_u5_proc);
    if (wayne_u5_kobj) {
        sysfs_remove_file(wayne_u5_kobj, &wayne_u5_attr.attr);
        kobject_put(wayne_u5_kobj);
    }
    input_unregister_device(wayne_u5_dev);
    pr_info("WAYNE ULTIMATE5 30.0: ultimate5 removido - timer %d - apertou %d - work %d - kthread %d\n", wayne_u5_timer_count, wayne_u5_press, wayne_u5_work_count, wayne_u5_kt_count);
}

module_init(wayne_u5_init);
module_exit(wayne_u5_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 30 - WAYNE ULTIMATE5 - tudo junto dos 30");
