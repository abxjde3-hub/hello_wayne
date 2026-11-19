#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>

static struct input_dev *wayne_kb_dev;

static void wayne_send_key(unsigned int code)
{
    input_report_key(wayne_kb_dev, code, 1);
    input_sync(wayne_kb_dev);
    input_report_key(wayne_kb_dev, code, 0);
    input_sync(wayne_kb_dev);
}

static int __init wayne_kb_init(void)
{
    int err;
    pr_info("WAYNE KEYBOARD 15.0: iniciando o teclado\n");

    wayne_kb_dev = input_allocate_device();
    if (!wayne_kb_dev)
        return -ENOMEM;

    wayne_kb_dev->name = "Wayne Keyboard Paw";
    wayne_kb_dev->phys = "wayne/keyboard0";
    wayne_kb_dev->id.bustype = BUS_VIRTUAL;
    wayne_kb_dev->id.vendor = 0x1234;
    wayne_kb_dev->id.product = 0x0015;
    wayne_kb_dev->id.version = 0x1500;

    __set_bit(EV_KEY, wayne_kb_dev->evbit);
    __set_bit(EV_SYN, wayne_kb_dev->evbit);
    __set_bit(KEY_W, wayne_kb_dev->keybit);
    __set_bit(KEY_A, wayne_kb_dev->keybit);
    __set_bit(KEY_Y, wayne_kb_dev->keybit);
    __set_bit(KEY_N, wayne_kb_dev->keybit);
    __set_bit(KEY_E, wayne_kb_dev->keybit);

    err = input_register_device(wayne_kb_dev);
    if (err) {
        input_free_device(wayne_kb_dev);
        pr_err("WAYNE KEYBOARD 15.0: falhou ao registrar\n");
        return err;
    }

    /* Teclado: digita W-A-Y-N-E */
    wayne_send_key(KEY_W);
    wayne_send_key(KEY_A);
    wayne_send_key(KEY_Y);
    wayne_send_key(KEY_N);
    wayne_send_key(KEY_E);

    pr_info("WAYNE KEYBOARD 15.0: digitado W-A-Y-N-E - Wayne Keyboard Paw\n");
    return 0;
}

static void __exit wayne_kb_exit(void)
{
    input_unregister_device(wayne_kb_dev);
    pr_info("WAYNE KEYBOARD 15.0: teclado removido\n");
}

module_init(wayne_kb_init);
module_exit(wayne_kb_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 15 - WAYNE KEYBOARD - digita WAYNE");
