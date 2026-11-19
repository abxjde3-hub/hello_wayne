#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>

static struct input_dev *wayne_u2_dev;

static int __init wayne_u2_init(void)
{
    int err;
    pr_info("WAYNE ULTIMATE2 12.0: iniciando o ultimate2\n");

    wayne_u2_dev = input_allocate_device();
    if (!wayne_u2_dev)
        return -ENOMEM;

    wayne_u2_dev->name = "Wayne Ultimate2 Paw";
    wayne_u2_dev->phys = "wayne/ultimate2";
    wayne_u2_dev->id.bustype = BUS_VIRTUAL;
    wayne_u2_dev->id.vendor = 0x1234;
    wayne_u2_dev->id.product = 0x0012;
    wayne_u2_dev->id.version = 0x1200;

    __set_bit(EV_KEY, wayne_u2_dev->evbit);
    __set_bit(EV_ABS, wayne_u2_dev->evbit);
    __set_bit(EV_REL, wayne_u2_dev->evbit);
    __set_bit(EV_SYN, wayne_u2_dev->evbit);
    __set_bit(BTN_LEFT, wayne_u2_dev->keybit);
    __set_bit(BTN_TOUCH, wayne_u2_dev->keybit);
    __set_bit(KEY_A, wayne_u2_dev->keybit);
    __set_bit(REL_X, wayne_u2_dev->relbit);
    __set_bit(REL_Y, wayne_u2_dev->relbit);

    input_set_abs_params(wayne_u2_dev, ABS_X, 0, 1080, 0, 0);
    input_set_abs_params(wayne_u2_dev, ABS_Y, 0, 2400, 0, 0);

    err = input_register_device(wayne_u2_dev);
    if (err) {
        input_free_device(wayne_u2_dev);
        pr_err("WAYNE ULTIMATE2 12.0: falhou ao registrar\n");
        return err;
    }

    /* ULTIMATE2: tecla + toque + clique + movimento */
    input_report_key(wayne_u2_dev, KEY_A, 1);
    input_sync(wayne_u2_dev);
    input_report_key(wayne_u2_dev, KEY_A, 0);
    input_sync(wayne_u2_dev);

    input_report_abs(wayne_u2_dev, ABS_X, 540);
    input_report_abs(wayne_u2_dev, ABS_Y, 1200);
    input_report_key(wayne_u2_dev, BTN_TOUCH, 1);
    input_sync(wayne_u2_dev);
    input_report_key(wayne_u2_dev, BTN_TOUCH, 0);
    input_sync(wayne_u2_dev);

    input_report_key(wayne_u2_dev, BTN_LEFT, 1);
    input_sync(wayne_u2_dev);
    input_report_key(wayne_u2_dev, BTN_LEFT, 0);
    input_sync(wayne_u2_dev);

    input_report_rel(wayne_u2_dev, REL_X, 10);
    input_report_rel(wayne_u2_dev, REL_Y, 10);
    input_sync(wayne_u2_dev);

    pr_info("WAYNE ULTIMATE2 12.0: ultimate2 enviado - KEY_A + toque 540,1200 + clique + move 10,10 - Wayne Ultimate2 Paw\n");
    return 0;
}

static void __exit wayne_u2_exit(void)
{
    input_unregister_device(wayne_u2_dev);
    pr_info("WAYNE ULTIMATE2 12.0: ultimate2 removido\n");
}

module_init(wayne_u2_init);
module_exit(wayne_u2_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 12 - WAYNE ULTIMATE2 - tudo junto");
