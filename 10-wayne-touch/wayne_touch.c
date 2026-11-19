#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>

static struct input_dev *wayne_touch_dev;

static int __init wayne_touch_init(void)
{
    int err;
    pr_info("WAYNE TOUCH 10.0: iniciando o toque\n");

    wayne_touch_dev = input_allocate_device();
    if (!wayne_touch_dev)
        return -ENOMEM;

    wayne_touch_dev->name = "Wayne Touch Paw";
    wayne_touch_dev->phys = "wayne/touch0";
    wayne_touch_dev->id.bustype = BUS_VIRTUAL;
    wayne_touch_dev->id.vendor = 0x1234;
    wayne_touch_dev->id.product = 0x0010;
    wayne_touch_dev->id.version = 0x1000;

    __set_bit(EV_KEY, wayne_touch_dev->evbit);
    __set_bit(EV_ABS, wayne_touch_dev->evbit);
    __set_bit(EV_SYN, wayne_touch_dev->evbit);
    __set_bit(BTN_TOUCH, wayne_touch_dev->keybit);

    input_set_abs_params(wayne_touch_dev, ABS_X, 0, 1080, 0, 0);
    input_set_abs_params(wayne_touch_dev, ABS_Y, 0, 2400, 0, 0);

    err = input_register_device(wayne_touch_dev);
    if (err) {
        input_free_device(wayne_touch_dev);
        pr_err("WAYNE TOUCH 10.0: falhou ao registrar\n");
        return err;
    }

    /* Manda um toque de verdade: toca em X=540, Y=1200 */
    input_report_abs(wayne_touch_dev, ABS_X, 540);
    input_report_abs(wayne_touch_dev, ABS_Y, 1200);
    input_report_key(wayne_touch_dev, BTN_TOUCH, 1);
    input_sync(wayne_touch_dev);
    input_report_key(wayne_touch_dev, BTN_TOUCH, 0);
    input_sync(wayne_touch_dev);

    pr_info("WAYNE TOUCH 10.0: toque em 540,1200 enviado - Wayne Touch Paw\n");
    return 0;
}

static void __exit wayne_touch_exit(void)
{
    input_unregister_device(wayne_touch_dev);
    pr_info("WAYNE TOUCH 10.0: toque removido\n");
}

module_init(wayne_touch_init);
module_exit(wayne_touch_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 10 - WAYNE TOUCH - manda toque");
