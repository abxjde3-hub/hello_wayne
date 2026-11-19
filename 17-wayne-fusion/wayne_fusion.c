#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>

static struct input_dev *wayne_fusion_dev;

static int __init wayne_fusion_init(void)
{
    int err;
    pr_info("WAYNE FUSION 17.0: iniciando a fusao\n");

    wayne_fusion_dev = input_allocate_device();
    if (!wayne_fusion_dev)
        return -ENOMEM;

    wayne_fusion_dev->name = "Wayne Fusion Paw";
    wayne_fusion_dev->phys = "wayne/fusion0";
    wayne_fusion_dev->id.bustype = BUS_VIRTUAL;
    wayne_fusion_dev->id.vendor = 0x1234;
    wayne_fusion_dev->id.product = 0x0017;
    wayne_fusion_dev->id.version = 0x1700;

    __set_bit(EV_KEY, wayne_fusion_dev->evbit);
    __set_bit(EV_ABS, wayne_fusion_dev->evbit);
    __set_bit(EV_REL, wayne_fusion_dev->evbit);
    __set_bit(EV_SYN, wayne_fusion_dev->evbit);
    __set_bit(KEY_W, wayne_fusion_dev->keybit);
    __set_bit(KEY_A, wayne_fusion_dev->keybit);
    __set_bit(KEY_Y, wayne_fusion_dev->keybit);
    __set_bit(KEY_N, wayne_fusion_dev->keybit);
    __set_bit(KEY_E, wayne_fusion_dev->keybit);
    __set_bit(BTN_LEFT, wayne_fusion_dev->keybit);
    __set_bit(BTN_A, wayne_fusion_dev->keybit);
    __set_bit(BTN_START, wayne_fusion_dev->keybit);
    __set_bit(REL_X, wayne_fusion_dev->relbit);
    __set_bit(REL_Y, wayne_fusion_dev->relbit);

    input_set_abs_params(wayne_fusion_dev, ABS_X, -32768, 32767, 0, 0);
    input_set_abs_params(wayne_fusion_dev, ABS_Y, -32768, 32767, 0, 0);
    input_set_abs_params(wayne_fusion_dev, ABS_Z, -1000, 1000, 0, 0);

    err = input_register_device(wayne_fusion_dev);
    if (err) {
        input_free_device(wayne_fusion_dev);
        pr_err("WAYNE FUSION 17.0: falhou ao registrar\n");
        return err;
    }

    /* FUSION: teclado + mouse + gamepad + sensor */
    input_report_key(wayne_fusion_dev, KEY_W, 1);
    input_sync(wayne_fusion_dev);
    input_report_key(wayne_fusion_dev, KEY_W, 0);
    input_sync(wayne_fusion_dev);

    input_report_rel(wayne_fusion_dev, REL_X, 20);
    input_report_rel(wayne_fusion_dev, REL_Y, 20);
    input_sync(wayne_fusion_dev);

    input_report_abs(wayne_fusion_dev, ABS_X, 5000);
    input_report_abs(wayne_fusion_dev, ABS_Y, -5000);
    input_report_abs(wayne_fusion_dev, ABS_Z, 800);
    input_sync(wayne_fusion_dev);

    input_report_key(wayne_fusion_dev, BTN_A, 1);
    input_sync(wayne_fusion_dev);
    input_report_key(wayne_fusion_dev, BTN_A, 0);
    input_sync(wayne_fusion_dev);

    pr_info("WAYNE FUSION 17.0: fusao enviada - KEY_W + mouse 20,20 + gamepad 5000,-5000 + sensor Z=800 + BTN_A - Wayne Fusion Paw\n");
    return 0;
}

static void __exit wayne_fusion_exit(void)
{
    input_unregister_device(wayne_fusion_dev);
    pr_info("WAYNE FUSION 17.0: fusao removida\n");
}

module_init(wayne_fusion_init);
module_exit(wayne_fusion_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 17 - WAYNE FUSION - tudo junto de novo");
