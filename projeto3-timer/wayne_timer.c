#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne");
MODULE_DESCRIPTION("Projeto 3 - WAYNE TIMER - Relogio dentro do Kernel");

static struct timer_list wayne_timer;
static unsigned long start_jiffies;
static unsigned long timer_count;

static void wayne_timer_callback(struct timer_list *t)
{
    timer_count++;
    mod_timer(&wayne_timer, jiffies + HZ);
}

static int wayne_timer_show(struct seq_file *m, void *v)
{
    unsigned long uptime_sec = (jiffies - start_jiffies) / HZ;
    seq_printf(m, "=== WAYNE TIMER 3.0 ===\n");
    seq_printf(m, "Segundos rodando: %lu s\n", uptime_sec);
    seq_printf(m, "Vezes que o timer disparou: %lu\n", timer_count);
    seq_printf(m, "Jiffies atual: %lu\n", jiffies);
    seq_printf(m, "HZ do sistema: %d\n", HZ);
    seq_printf(m, "Status: TIMER ATIVO - disparando a cada 1s\n");
    return 0;
}

static int wayne_timer_open(struct inode *inode, struct file *file)
{
    return single_open(file, wayne_timer_show, NULL);
}

static const struct proc_ops wayne_timer_ops = {
    .proc_open = wayne_timer_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init wayne_timer_init(void)
{
    start_jiffies = jiffies;
    timer_count = 0;
    proc_create("wayne_timer", 0, NULL, &wayne_timer_ops);
    timer_setup(&wayne_timer, wayne_timer_callback, 0);
    mod_timer(&wayne_timer, jiffies + HZ);
    pr_info("WAYNE TIMER carregado - timer a cada 1s\n");
    return 0;
}

static void __exit wayne_timer_exit(void)
{
    timer_delete_sync(&wayne_timer);
    remove_proc_entry("wayne_timer", NULL);
    pr_info("WAYNE TIMER removido - disparou %lu vezes\n", timer_count);
}

module_init(wayne_timer_init);
module_exit(wayne_timer_exit);
