#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>

static struct input_dev *wayne_combo_dev;

static int __init wayne_combo_init(void)
{
    int err;
    pr_info("WAYNE COMBO 11.0: iniciando o combo\n");

    wayne_combo_dev = input_allocate_device();
    if (!wayne_combo_dev)
        return -ENOMEM;

    wayne_combo_dev->name = "Wayne Combo Paw";
    wayne_combo_dev->phys = "wayne/combo0";
    wayne_combo_dev->id.bustype = BUS_VIRTUAL;
    wayne_combo_dev->id.vendor = 0x1234;
    wayne_combo_dev->id.product = 0x0011;
    wayne_combo_dev->id.version = 0x1100;

    __set_bit(EV_KEY, wayne_combo_dev->evbit);
    __set_bit(EV_ABS, wayne_combo_dev->evbit);
    __set_bit(EV_SYN, wayne_combo_dev->evbit);
    __set_bit(BTN_LEFT, wayne_combo_dev->keybit);
    __set_bit(BTN_TOUCH, wayne_combo_dev->keybit);
    __set_bit(KEY_A, wayne_combo_dev->keybit);

    input_set_abs_params(wayne_combo_dev, ABS_X, 0, 1080, 0, 0);
    input_set_abs_params(wayne_combo_dev, ABS_Y, 0, 2400, 0, 0);

    err = input_register_device(wayne_combo_dev);
    if (err) {
        input_free_device(wayne_combo_dev);
        pr_err("WAYNE COMBO 11.0: falhou ao registrar\n");
        return err;
    }

    /* Combo: tecla + toque + clique */
    input_report_key(wayne_combo_dev, KEY_A, 1);
    input_sync(wayne_combo_dev);
    input_report_key(wayne_combo_dev, KEY_A, 0);
    input_sync(wayne_combo_dev);

    input_report_abs(wayne_combo_dev, ABS_X, 540);
    input_report_abs(wayne_combo_dev, ABS_Y, 1200);
    input_report_key(wayne_combo_dev, BTN_TOUCH, 1);
    input_sync(wayne_combo_dev);
    input_report_key(wayne_combo_dev, BTN_TOUCH, 0);
    input_sync(wayne_combo_dev);

    input_report_key(wayne_combo_dev, BTN_LEFT, 1);
    input_sync(wayne_combo_dev);
    input_report_key(wayne_combo_dev, BTN_LEFT, 0);
    input_sync(wayne_combo_dev);

    pr_info("WAYNE COMBO 11.0: combo enviado - KEY_A + toque 540,1200 + clique - Wayne Combo Paw\n");
    return 0;
}

static void __exit wayne_combo_exit(void)
{
    input_unregister_device(wayne_combo_dev);
    pr_info("WAYNE COMBO 11.0: combo removido\n");
}

module_init(wayne_combo_init);
module_exit(wayne_combo_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 11 - WAYNE COMBO - tecla + toque + clique");
