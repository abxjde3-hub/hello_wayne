#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>

static struct input_dev *wayne_pad_dev;

static int __init wayne_pad_init(void)
{
    int err;
    pr_info("WAYNE GAMEPAD 13.0: iniciando o gamepad\n");

    wayne_pad_dev = input_allocate_device();
    if (!wayne_pad_dev)
        return -ENOMEM;

    wayne_pad_dev->name = "Wayne Gamepad Paw";
    wayne_pad_dev->phys = "wayne/gamepad0";
    wayne_pad_dev->id.bustype = BUS_VIRTUAL;
    wayne_pad_dev->id.vendor = 0x1234;
    wayne_pad_dev->id.product = 0x0013;
    wayne_pad_dev->id.version = 0x1300;

    __set_bit(EV_KEY, wayne_pad_dev->evbit);
    __set_bit(EV_ABS, wayne_pad_dev->evbit);
    __set_bit(EV_SYN, wayne_pad_dev->evbit);
    __set_bit(BTN_A, wayne_pad_dev->keybit);
    __set_bit(BTN_B, wayne_pad_dev->keybit);
    __set_bit(BTN_X, wayne_pad_dev->keybit);
    __set_bit(BTN_Y, wayne_pad_dev->keybit);
    __set_bit(BTN_START, wayne_pad_dev->keybit);

    input_set_abs_params(wayne_pad_dev, ABS_X, -32768, 32767, 0, 0);
    input_set_abs_params(wayne_pad_dev, ABS_Y, -32768, 32767, 0, 0);

    err = input_register_device(wayne_pad_dev);
    if (err) {
        input_free_device(wayne_pad_dev);
        pr_err("WAYNE GAMEPAD 13.0: falhou ao registrar\n");
        return err;
    }

    /* Gamepad: move o analógico e aperta A + START */
    input_report_abs(wayne_pad_dev, ABS_X, 10000);
    input_report_abs(wayne_pad_dev, ABS_Y, -10000);
    input_sync(wayne_pad_dev);

    input_report_key(wayne_pad_dev, BTN_A, 1);
    input_sync(wayne_pad_dev);
    input_report_key(wayne_pad_dev, BTN_A, 0);
    input_sync(wayne_pad_dev);

    input_report_key(wayne_pad_dev, BTN_START, 1);
    input_sync(wayne_pad_dev);
    input_report_key(wayne_pad_dev, BTN_START, 0);
    input_sync(wayne_pad_dev);

    pr_info("WAYNE GAMEPAD 13.0: gamepad enviado - ABS 10000,-10000 + BTN_A + BTN_START - Wayne Gamepad Paw\n");
    return 0;
}

static void __exit wayne_pad_exit(void)
{
    input_unregister_device(wayne_pad_dev);
    pr_info("WAYNE GAMEPAD 13.0: gamepad removido\n");
}

module_init(wayne_pad_init);
module_exit(wayne_pad_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 13 - WAYNE GAMEPAD - joystick + botoes");
