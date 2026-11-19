#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>

static struct input_dev *wayne_input_dev;

static int __init wayne_input_init(void)
{
    int err;
    pr_info("WAYNE INPUT 8.0: iniciando a pata\n");

    wayne_input_dev = input_allocate_device();
    if (!wayne_input_dev)
        return -ENOMEM;

    wayne_input_dev->name = "Wayne Input Paw";
    wayne_input_dev->phys = "wayne/input0";
    wayne_input_dev->id.bustype = BUS_VIRTUAL;
    wayne_input_dev->id.vendor = 0x1234;
    wayne_input_dev->id.product = 0x0008;
    wayne_input_dev->id.version = 0x0800;

    __set_bit(EV_KEY, wayne_input_dev->evbit);
    __set_bit(BTN_LEFT, wayne_input_dev->keybit);
    __set_bit(BTN_RIGHT, wayne_input_dev->keybit);
    __set_bit(KEY_A, wayne_input_dev->keybit);

    err = input_register_device(wayne_input_dev);
    if (err) {
        input_free_device(wayne_input_dev);
        pr_err("WAYNE INPUT 8.0: falhou ao registrar\n");
        return err;
    }

    pr_info("WAYNE INPUT 8.0: pata registrada - Wayne Input Paw\n");
    return 0;
}

static void __exit wayne_input_exit(void)
{
    input_unregister_device(wayne_input_dev);
    pr_info("WAYNE INPUT 8.0: pata removida\n");
}

module_init(wayne_input_init);
module_exit(wayne_input_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 8 - WAYNE INPUT - a pata");
