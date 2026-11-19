#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

static struct input_dev *wayne_timer_dev;
static struct timer_list wayne_timer;
static int wayne_count;

static void wayne_timer_fn(struct timer_list *t)
{
    wayne_count++;
    input_report_key(wayne_timer_dev, KEY_A, 1);
    input_sync(wayne_timer_dev);
    input_report_key(wayne_timer_dev, KEY_A, 0);
    input_sync(wayne_timer_dev);
    pr_info("WAYNE TIMER 18.0: batida %d - KEY_A - Wayne Timer Paw\n", wayne_count);
    mod_timer(&wayne_timer, jiffies + msecs_to_jiffies(2000));
}

static int __init wayne_timer_init(void)
{
    int err;
    pr_info("WAYNE TIMER 18.0: iniciando o timer\n");

    wayne_timer_dev = input_allocate_device();
    if (!wayne_timer_dev)
        return -ENOMEM;

    wayne_timer_dev->name = "Wayne Timer Paw";
    wayne_timer_dev->phys = "wayne/timer0";
    wayne_timer_dev->id.bustype = BUS_VIRTUAL;
    wayne_timer_dev->id.vendor = 0x1234;
    wayne_timer_dev->id.product = 0x0018;
    wayne_timer_dev->id.version = 0x1800;

    __set_bit(EV_KEY, wayne_timer_dev->evbit);
    __set_bit(EV_SYN, wayne_timer_dev->evbit);
    __set_bit(KEY_A, wayne_timer_dev->keybit);

    err = input_register_device(wayne_timer_dev);
    if (err) {
        input_free_device(wayne_timer_dev);
        pr_err("WAYNE TIMER 18.0: falhou ao registrar\n");
        return err;
    }

    wayne_count = 0;
    timer_setup(&wayne_timer, wayne_timer_fn, 0);
    mod_timer(&wayne_timer, jiffies + msecs_to_jiffies(2000));

    pr_info("WAYNE TIMER 18.0: timer ligado - vai bater a cada 2s - Wayne Timer Paw\n");
    return 0;
}

static void __exit wayne_timer_exit(void)
{
    timer_delete_sync(&wayne_timer);
    input_unregister_device(wayne_timer_dev);
    pr_info("WAYNE TIMER 18.0: timer removido - bateu %d vezes\n", wayne_count);
}

module_init(wayne_timer_init);
module_exit(wayne_timer_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 18 - WAYNE TIMER - bate a cada 2s");
