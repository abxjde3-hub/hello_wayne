#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>
#include <linux/ktime.h>
#include <linux/sched.h>
#include <linux/sched/loadavg.h>

#define PROC_NAME "wayne_monitor"

static int wayne_show(struct seq_file *m, void *v)
{
    struct sysinfo info;
    struct task_struct *p;
    u64 uptime;
    unsigned long total_mem, free_mem;
    int total_procs = 0, running_procs = 0;

    si_meminfo(&info);
    total_mem = info.totalram * info.mem_unit / 1024 / 1024;
    free_mem = info.freeram * info.mem_unit / 1024 / 1024;
    uptime = ktime_get_boottime_seconds();

    // Conta processos - precisa travar a lista pra ler com seguranca
    rcu_read_lock();
    for_each_process(p) {
        total_procs++;
        if (p->__state == TASK_RUNNING)
            running_procs++;
    }
    rcu_read_unlock();

    seq_printf(m, "=== WAYNE MONITOR 2.0 - Feito por voce ===\n");
    seq_printf(m, "Uptime: %llu segundos (%llu minutos)\n", uptime, uptime / 60);
    seq_printf(m, "CPU online: %d nucleos\n", num_online_cpus());
    seq_printf(m, "Jiffies: %lu\n", jiffies);
    seq_printf(m, "Carga CPU (load): %lu.%02lu  %lu.%02lu  %lu.%02lu\n",
        LOAD_INT(avenrun[0]), LOAD_FRAC(avenrun[0]),
        LOAD_INT(avenrun[1]), LOAD_FRAC(avenrun[1]),
        LOAD_INT(avenrun[2]), LOAD_FRAC(avenrun[2]));
    seq_printf(m, "Processos totais: %d\n", total_procs);
    seq_printf(m, "Processos rodando agora: %d\n", running_procs);
    seq_printf(m, "Memoria total: %lu MB\n", total_mem);
    seq_printf(m, "Memoria livre: %lu MB\n", free_mem);
    seq_printf(m, "Memoria usada: %lu MB\n", total_mem - free_mem);
    return 0;
}

static int wayne_open(struct inode *inode, struct file *file)
{
    return single_open(file, wayne_show, NULL);
}

static const struct proc_ops wayne_ops = {
    .proc_open = wayne_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init wayne_init(void)
{
    proc_create(PROC_NAME, 0, NULL, &wayne_ops);
    pr_info("wayne_monitor 2.0: carregado!\n");
    return 0;
}

static void __exit wayne_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    pr_info("wayne_monitor: removido\n");
}

module_init(wayne_init);
module_exit(wayne_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Voce");
MODULE_DESCRIPTION("Monitor do Kernel 2.0 - Projeto 2");
