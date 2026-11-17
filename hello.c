#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/ktime.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mestre Wayne Final Boss");
MODULE_VERSION("7.4");

#define PROC_NAME "wayne"
#define DEV_NAME "wayne"
#define MSG_SIZE 1024
#define DEV_BUF 8192

static char wayne_msg[MSG_SIZE] = "FINAL BOSS LEVEL 7!";
static dev_t dev_num;
static struct cdev wayne_cdev;
static struct class *wayne_class;
static struct kobject *wayne_kobj;

static ssize_t wayne_read_common(char *buf){
    struct sysinfo si; u64 uptime; struct task_struct *task; int len=0;
    si_meminfo(&si); uptime = ktime_get_boottime_seconds();
    len += snprintf(buf+len, DEV_BUF-len,
        "========== WAYNE KERNEL 7.4 FINAL BOSS ==========\n"
        "Uptime %llu seg | RAM %lu MB | %s PID %d\n"
        "Msg: %s\nTop 10 processos:\n",
        uptime, (si.freeram*si.mem_unit)/1024/1024,
        current->comm, current->pid, wayne_msg);
    int i=0;
    for_each_process(task){
        if(i++>10) break;
        len += snprintf(buf+len, DEV_BUF-len, " %d - %s\n", task->pid, task->comm);
    }
    len += snprintf(buf+len, DEV_BUF-len, "==============================================\n");
    return len;
}
static ssize_t wayne_dev_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos){
    char *buf = kmalloc(DEV_BUF, GFP_KERNEL); if(!buf) return -ENOMEM;
    int len = wayne_read_common(buf);
    int ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);
    kfree(buf); return ret;
}
static ssize_t wayne_dev_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos){
    if(count>=MSG_SIZE) count=MSG_SIZE-1;
    if(copy_from_user(wayne_msg, ubuf, count)) return -EFAULT;
    wayne_msg[count]='\0';
    if(count>0 && wayne_msg[count-1]=='\n') wayne_msg[count-1]='\0';
    return count;
}
static struct file_operations wayne_fops = {.owner=THIS_MODULE,.read=wayne_dev_read,.write=wayne_dev_write,};
static ssize_t wayne_proc_read(struct file *f, char __user *u, size_t c, loff_t *p){ return wayne_dev_read(f,u,c,p); }
static ssize_t wayne_proc_write(struct file *f, const char __user *u, size_t c, loff_t *p){ return wayne_dev_write(f,u,c,p); }
static const struct proc_ops wayne_proc_ops = {.proc_read=wayne_proc_read,.proc_write=wayne_proc_write,};
static ssize_t msg_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){ return sysfs_emit(buf, "%s\n", wayne_msg); }
static ssize_t msg_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
    size_t len = min(count, (size_t)(MSG_SIZE-1));
    strscpy(wayne_msg, buf, len+1);
    if(len>0 && wayne_msg[len-1]=='\n') wayne_msg[len-1]='\0';
    return count;
}
static ssize_t info_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){ return wayne_read_common(buf); }
static struct kobj_attribute wayne_msg_attr = __ATTR(message, 0644, msg_show, msg_store);
static struct kobj_attribute wayne_info_attr = __ATTR(info, 0444, info_show, NULL);

static int __init wayne_init(void){
    int ret;
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME); if(ret<0) return ret;
    cdev_init(&wayne_cdev, &wayne_fops);
    ret = cdev_add(&wayne_cdev, dev_num, 1); if(ret<0){ unregister_chrdev_region(dev_num,1); return ret; }
    wayne_class = class_create(DEV_NAME);
    if(IS_ERR(wayne_class)){ cdev_del(&wayne_cdev); unregister_chrdev_region(dev_num,1); return PTR_ERR(wayne_class); }
    device_create(wayne_class, NULL, dev_num, NULL, DEV_NAME);
    proc_create(PROC_NAME, 0666, NULL, &wayne_proc_ops);
    wayne_kobj = kobject_create_and_add("wayne", kernel_kobj);
    if(wayne_kobj){
        ret = sysfs_create_file(wayne_kobj, &wayne_msg_attr.attr);
        ret = sysfs_create_file(wayne_kobj, &wayne_info_attr.attr);
    }
    printk(KERN_INFO "FINAL BOSS CRIADO MAJOR %d\n", MAJOR(dev_num));
    return 0;
}
static void __exit wayne_exit(void){
    if(wayne_kobj){
        sysfs_remove_file(wayne_kobj, &wayne_msg_attr.attr);
        sysfs_remove_file(wayne_kobj, &wayne_info_attr.attr);
        kobject_put(wayne_kobj);
    }
    device_destroy(wayne_class, dev_num);
    class_destroy(wayne_class);
    cdev_del(&wayne_cdev);
    unregister_chrdev_region(dev_num, 1);
    remove_proc_entry(PROC_NAME, NULL);
}
module_init(wayne_init);
module_exit(wayne_exit);
