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

static struct input_dev *wayne_u4_dev;
static struct timer_list wayne_u4_timer;
static struct kobject *wayne_u4_kobj;
static struct proc_dir_entry *wayne_u4_proc;
static struct dentry *wayne_u4_dbg_dir;
static struct dentry *wayne_u4_dbg_file;
static struct work_struct wayne_u4_work;
static struct task_struct *wayne_u4_task;
static int wayne_u4_timer_count;
static int wayne_u4_press;
static int wayne_u4_work_count;
static int wayne_u4_kt_count;

static void wayne_u4_timer_fn(struct timer_list *t)
{
    wayne_u4_timer_count++;
    input_report_key(wayne_u4_dev, KEY_A, 1);
    input_sync(wayne_u4_dev);
    input_report_key(wayne_u4_dev, KEY_A, 0);
    input_sync(wayne_u4_dev);
    pr_info("WAYNE ULTIMATE4 25.0: batida timer %d - KEY_A\n", wayne_u4_timer_count);
    mod_timer(&wayne_u4_timer, jiffies + msecs_to_jiffies(2000));
}

static void wayne_u4_work_fn(struct work_struct *work)
{
    wayne_u4_work_count++;
    input_report_key(wayne_u4_dev, KEY_A, 1);
    input_sync(wayne_u4_dev);
    input_report_key(wayne_u4_dev, KEY_A, 0);
    input_sync(wayne_u4_dev);
    pr_info("WAYNE ULTIMATE4 25.0: trabalho %d em segundo plano - KEY_A\n", wayne_u4_work_count);
}

static int wayne_u4_kt_fn(void *data)
{
    while (!kthread_should_stop()) {
        wayne_u4_kt_count++;
        input_report_key(wayne_u4_dev, KEY_A, 1);
        input_sync(wayne_u4_dev);
        input_report_key(wayne_u4_dev, KEY_A, 0);
        input_sync(wayne_u4_dev);
        pr_info("WAYNE ULTIMATE4 25.0: volta kthread %d - KEY_A\n", wayne_u4_kt_count);
        msleep(5000);
    }
    return 0;
}

static void wayne_u4_fire(void)
{
    wayne_u4_press++;
    input_report_key(wayne_u4_dev, KEY_W, 1);
    input_sync(wayne_u4_dev);
    input_report_key(wayne_u4_dev, KEY_W, 0);
    input_sync(wayne_u4_dev);
    input_report_rel(wayne_u4_dev, REL_X, 20);
    input_report_rel(wayne_u4_dev, REL_Y, 20);
    input_sync(wayne_u4_dev);
    input_report_abs(wayne_u4_dev, ABS_X, 5000);
    input_report_abs(wayne_u4_dev, ABS_Y, -5000);
    input_report_abs(wayne_u4_dev, ABS_Z, 800);
    input_sync(wayne_u4_dev);
    input_report_key(wayne_u4_dev, BTN_A, 1);
    input_sync(wayne_u4_dev);
    input_report_key(wayne_u4_dev, BTN_A, 0);
    input_sync(wayne_u4_dev);
    pr_info("WAYNE ULTIMATE4 25.0: aperto %d - KEY_W + mouse + gamepad + sensor + BTN_A\n", wayne_u4_press);
}

static ssize_t wayne_u4_sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "Wayne Ultimate4 Paw - timer %d - apertos %d - work %d - kthread %d\n", wayne_u4_timer_count, wayne_u4_press, wayne_u4_work_count, wayne_u4_kt_count);
}

static ssize_t wayne_u4_sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    wayne_u4_fire();
    return count;
}

static struct kobj_attribute wayne_u4_attr = __ATTR(press, 0664, wayne_u4_sysfs_show, wayne_u4_sysfs_store);

static int wayne_u4_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Wayne Ultimate4 Paw - timer %d - apertos %d - work %d - kthread %d\n", wayne_u4_timer_count, wayne_u4_press, wayne_u4_work_count, wayne_u4_kt_count);
    return 0;
}

static int wayne_u4_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, wayne_u4_proc_show, NULL);
}

static ssize_t wayne_u4_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    wayne_u4_fire();
    return count;
}

static const struct proc_ops wayne_u4_proc_ops = {
    .proc_open = wayne_u4_proc_open,
    .proc_read = seq_read,
    .proc_write = wayne_u4_proc_write,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int wayne_u4_dbg_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Wayne Ultimate4 Paw - timer %d - apertos %d - work %d - kthread %d\n", wayne_u4_timer_count, wayne_u4_press, wayne_u4_work_count, wayne_u4_kt_count);
    return 0;
}

static int wayne_u4_dbg_open(struct inode *inode, struct file *file)
{
    return single_open(file, wayne_u4_dbg_show, NULL);
}

static ssize_t wayne_u4_dbg_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    wayne_u4_fire();
    return count;
}

static const struct file_operations wayne_u4_dbg_fops = {
    .owner = THIS_MODULE,
    .open = wayne_u4_dbg_open,
    .read = seq_read,
    .write = wayne_u4_dbg_write,
    .llseek = seq_lseek,
    .release = single_release,
};

static int __init wayne_u4_init(void)
{
    int err;
    pr_info("WAYNE ULTIMATE4 25.0: iniciando o ultimate4\n");

    wayne_u4_dev = input_allocate_device();
    if (!wayne_u4_dev)
        return -ENOMEM;

    wayne_u4_dev->name = "Wayne Ultimate4 Paw";
    wayne_u4_dev->phys = "wayne/ultimate4";
    wayne_u4_dev->id.bustype = BUS_VIRTUAL;
    wayne_u4_dev->id.vendor = 0x1234;
    wayne_u4_dev->id.product = 0x0025;
    wayne_u4_dev->id.version = 0x2500;

    __set_bit(EV_KEY, wayne_u4_dev->evbit);
    __set_bit(EV_ABS, wayne_u4_dev->evbit);
    __set_bit(EV_REL, wayne_u4_dev->evbit);
    __set_bit(EV_SYN, wayne_u4_dev->evbit);
    __set_bit(KEY_W, wayne_u4_dev->keybit);
    __set_bit(KEY_A, wayne_u4_dev->keybit);
    __set_bit(KEY_Y, wayne_u4_dev->keybit);
    __set_bit(KEY_N, wayne_u4_dev->keybit);
    __set_bit(KEY_E, wayne_u4_dev->keybit);
    __set_bit(BTN_LEFT, wayne_u4_dev->keybit);
    __set_bit(BTN_A, wayne_u4_dev->keybit);
    __set_bit(BTN_START, wayne_u4_dev->keybit);
    __set_bit(REL_X, wayne_u4_dev->relbit);
    __set_bit(REL_Y, wayne_u4_dev->relbit);

    input_set_abs_params(wayne_u4_dev, ABS_X, -32768, 32767, 0, 0);
    input_set_abs_params(wayne_u4_dev, ABS_Y, -32768, 32767, 0, 0);
    input_set_abs_params(wayne_u4_dev, ABS_Z, -1000, 1000, 0, 0);

    err = input_register_device(wayne_u4_dev);
    if (err) {
        input_free_device(wayne_u4_dev);
        return err;
    }

    wayne_u4_timer_count = 0;
    wayne_u4_press = 0;
    wayne_u4_work_count = 0;
    wayne_u4_kt_count = 0;

    wayne_u4_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if (wayne_u4_kobj)
        sysfs_create_file(wayne_u4_kobj, &wayne_u4_attr.attr);

    wayne_u4_proc = proc_create("wayne", 0666, NULL, &wayne_u4_proc_ops);

    wayne_u4_dbg_dir = debugfs_create_dir("wayne", NULL);
    if (wayne_u4_dbg_dir)
        wayne_u4_dbg_file = debugfs_create_file("press", 0666, wayne_u4_dbg_dir, NULL, &wayne_u4_dbg_fops);

    timer_setup(&wayne_u4_timer, wayne_u4_timer_fn, 0);
    mod_timer(&wayne_u4_timer, jiffies + msecs_to_jiffies(2000));

    INIT_WORK(&wayne_u4_work, wayne_u4_work_fn);
    schedule_work(&wayne_u4_work);

    wayne_u4_task = kthread_run(wayne_u4_kt_fn, NULL, "wayne_ultimate4");

    pr_info("WAYNE ULTIMATE4 25.0: ultimate4 ligado - timer + sysfs + proc + debugfs + workqueue + kthread - Wayne Ultimate4 Paw\n");
    return 0;
}

static void __exit wayne_u4_exit(void)
{
    if (wayne_u4_task)
        kthread_stop(wayne_u4_task);
    timer_delete_sync(&wayne_u4_timer);
    flush_work(&wayne_u4_work);
    cancel_work_sync(&wayne_u4_work);
    if (wayne_u4_dbg_dir)
        debugfs_remove_recursive(wayne_u4_dbg_dir);
    if (wayne_u4_proc)
        proc_remove(wayne_u4_proc);
    if (wayne_u4_kobj) {
        sysfs_remove_file(wayne_u4_kobj, &wayne_u4_attr.attr);
        kobject_put(wayne_u4_kobj);
    }
    input_unregister_device(wayne_u4_dev);
    pr_info("WAYNE ULTIMATE4 25.0: ultimate4 removido - timer %d - apertou %d - work %d - kthread %d\n", wayne_u4_timer_count, wayne_u4_press, wayne_u4_work_count, wayne_u4_kt_count);
}

module_init(wayne_u4_init);
module_exit(wayne_u4_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 25 - WAYNE ULTIMATE4 - tudo junto");
