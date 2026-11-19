#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/workqueue.h>

static struct input_dev *wayne_wq_dev;
static struct work_struct wayne_work;
static int wayne_wq_count;

static void wayne_work_fn(struct work_struct *work)
{
    wayne_wq_count++;
    input_report_key(wayne_wq_dev, KEY_A, 1);
    input_sync(wayne_wq_dev);
    input_report_key(wayne_wq_dev, KEY_A, 0);
    input_sync(wayne_wq_dev);
    pr_info("WAYNE WORKQUEUE 22.0: trabalho %d feito em segundo plano - KEY_A - Wayne Workqueue Paw\n", wayne_wq_count);
}

static int __init wayne_wq_init(void)
{
    int err;
    pr_info("WAYNE WORKQUEUE 22.0: iniciando a workqueue\n");

    wayne_wq_dev = input_allocate_device();
    if (!wayne_wq_dev)
        return -ENOMEM;

    wayne_wq_dev->name = "Wayne Workqueue Paw";
    wayne_wq_dev->phys = "wayne/workqueue0";
    wayne_wq_dev->id.bustype = BUS_VIRTUAL;
    wayne_wq_dev->id.vendor = 0x1234;
    wayne_wq_dev->id.product = 0x0022;
    wayne_wq_dev->id.version = 0x2200;

    __set_bit(EV_KEY, wayne_wq_dev->evbit);
    __set_bit(EV_SYN, wayne_wq_dev->evbit);
    __set_bit(KEY_A, wayne_wq_dev->keybit);

    err = input_register_device(wayne_wq_dev);
    if (err) {
        input_free_device(wayne_wq_dev);
        pr_err("WAYNE WORKQUEUE 22.0: falhou ao registrar\n");
        return err;
    }

    wayne_wq_count = 0;
    INIT_WORK(&wayne_work, wayne_work_fn);
    schedule_work(&wayne_work);
    schedule_work(&wayne_work);
    schedule_work(&wayne_work);

    pr_info("WAYNE WORKQUEUE 22.0: 3 trabalhos agendados - Wayne Workqueue Paw\n");
    return 0;
}

static void __exit wayne_wq_exit(void)
{
    flush_work(&wayne_work);
    cancel_work_sync(&wayne_work);
    input_unregister_device(wayne_wq_dev);
    pr_info("WAYNE WORKQUEUE 22.0: workqueue removida - fez %d trabalhos\n", wayne_wq_count);
}

module_init(wayne_wq_init);
module_exit(wayne_wq_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 22 - WAYNE WORKQUEUE - trabalho em segundo plano");
