#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>

static struct input_dev *wayne_event_dev;

static int __init wayne_event_init(void)
{
    int err;
    pr_info("WAYNE EVENT 9.0: iniciando o evento\n");

    wayne_event_dev = input_allocate_device();
    if (!wayne_event_dev)
        return -ENOMEM;

    wayne_event_dev->name = "Wayne Event Paw";
    wayne_event_dev->phys = "wayne/event0";
    wayne_event_dev->id.bustype = BUS_VIRTUAL;
    wayne_event_dev->id.vendor = 0x1234;
    wayne_event_dev->id.product = 0x0009;
    wayne_event_dev->id.version = 0x0900;

    __set_bit(EV_KEY, wayne_event_dev->evbit);
    __set_bit(EV_SYN, wayne_event_dev->evbit);
    __set_bit(BTN_LEFT, wayne_event_dev->keybit);
    __set_bit(KEY_A, wayne_event_dev->keybit);

    err = input_register_device(wayne_event_dev);
    if (err) {
        input_free_device(wayne_event_dev);
        pr_err("WAYNE EVENT 9.0: falhou ao registrar\n");
        return err;
    }

    /* Manda um evento de verdade: aperta e solta KEY_A */
    input_report_key(wayne_event_dev, KEY_A, 1);
    input_sync(wayne_event_dev);
    input_report_key(wayne_event_dev, KEY_A, 0);
    input_sync(wayne_event_dev);

    pr_info("WAYNE EVENT 9.0: evento KEY_A enviado - Wayne Event Paw\n");
    return 0;
}

static void __exit wayne_event_exit(void)
{
    input_unregister_device(wayne_event_dev);
    pr_info("WAYNE EVENT 9.0: evento removido\n");
}

module_init(wayne_event_init);
module_exit(wayne_event_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 9 - WAYNE EVENT - manda evento");
