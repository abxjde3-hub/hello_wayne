#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static struct input_dev *wayne_kt_dev;
static struct task_struct *wayne_kt_task;
static int wayne_kt_count;

static int wayne_kt_fn(void *data)
{
    while (!kthread_should_stop()) {
        wayne_kt_count++;
        input_report_key(wayne_kt_dev, KEY_A, 1);
        input_sync(wayne_kt_dev);
        input_report_key(wayne_kt_dev, KEY_A, 0);
        input_sync(wayne_kt_dev);
        pr_info("WAYNE KTHREAD 23.0: volta %d da thread - KEY_A - Wayne Kthread Paw\n", wayne_kt_count);
        msleep(2000);
    }
    return 0;
}

static int __init wayne_kt_init(void)
{
    int err;
    pr_info("WAYNE KTHREAD 23.0: iniciando a kthread\n");

    wayne_kt_dev = input_allocate_device();
    if (!wayne_kt_dev)
        return -ENOMEM;

    wayne_kt_dev->name = "Wayne Kthread Paw";
    wayne_kt_dev->phys = "wayne/kthread0";
    wayne_kt_dev->id.bustype = BUS_VIRTUAL;
    wayne_kt_dev->id.vendor = 0x1234;
    wayne_kt_dev->id.product = 0x0023;
    wayne_kt_dev->id.version = 0x2300;

    __set_bit(EV_KEY, wayne_kt_dev->evbit);
    __set_bit(EV_SYN, wayne_kt_dev->evbit);
    __set_bit(KEY_A, wayne_kt_dev->keybit);

    err = input_register_device(wayne_kt_dev);
    if (err) {
        input_free_device(wayne_kt_dev);
        pr_err("WAYNE KTHREAD 23.0: falhou ao registrar\n");
        return err;
    }

    wayne_kt_count = 0;
    wayne_kt_task = kthread_run(wayne_kt_fn, NULL, "wayne_kthread");
    if (IS_ERR(wayne_kt_task)) {
        input_unregister_device(wayne_kt_dev);
        return PTR_ERR(wayne_kt_task);
    }

    pr_info("WAYNE KTHREAD 23.0: kthread ligada - vai dar volta a cada 2s - Wayne Kthread Paw\n");
    return 0;
}

static void __exit wayne_kt_exit(void)
{
    if (wayne_kt_task)
        kthread_stop(wayne_kt_task);
    input_unregister_device(wayne_kt_dev);
    pr_info("WAYNE KTHREAD 23.0: kthread parada - deu %d voltas\n", wayne_kt_count);
}

module_init(wayne_kt_init);
module_exit(wayne_kt_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 23 - WAYNE KTHREAD - thread viva");
