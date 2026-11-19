#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>

static struct input_dev *wayne_mouse_dev;

static int __init wayne_mouse_init(void)
{
    int err;
    pr_info("WAYNE MOUSE 16.0: iniciando o mouse\n");

    wayne_mouse_dev = input_allocate_device();
    if (!wayne_mouse_dev)
        return -ENOMEM;

    wayne_mouse_dev->name = "Wayne Mouse Paw";
    wayne_mouse_dev->phys = "wayne/mouse0";
    wayne_mouse_dev->id.bustype = BUS_VIRTUAL;
    wayne_mouse_dev->id.vendor = 0x1234;
    wayne_mouse_dev->id.product = 0x0016;
    wayne_mouse_dev->id.version = 0x1600;

    __set_bit(EV_KEY, wayne_mouse_dev->evbit);
    __set_bit(EV_REL, wayne_mouse_dev->evbit);
    __set_bit(EV_SYN, wayne_mouse_dev->evbit);
    __set_bit(BTN_LEFT, wayne_mouse_dev->keybit);
    __set_bit(BTN_RIGHT, wayne_mouse_dev->keybit);
    __set_bit(REL_X, wayne_mouse_dev->relbit);
    __set_bit(REL_Y, wayne_mouse_dev->relbit);
    __set_bit(REL_WHEEL, wayne_mouse_dev->relbit);

    err = input_register_device(wayne_mouse_dev);
    if (err) {
        input_free_device(wayne_mouse_dev);
        pr_err("WAYNE MOUSE 16.0: falhou ao registrar\n");
        return err;
    }

    /* Mouse: move 50,50 + clica esquerdo + rola 1 */
    input_report_rel(wayne_mouse_dev, REL_X, 50);
    input_report_rel(wayne_mouse_dev, REL_Y, 50);
    input_sync(wayne_mouse_dev);

    input_report_key(wayne_mouse_dev, BTN_LEFT, 1);
    input_sync(wayne_mouse_dev);
    input_report_key(wayne_mouse_dev, BTN_LEFT, 0);
    input_sync(wayne_mouse_dev);

    input_report_rel(wayne_mouse_dev, REL_WHEEL, 1);
    input_sync(wayne_mouse_dev);

    pr_info("WAYNE MOUSE 16.0: mouse enviado - move 50,50 + clique + roda 1 - Wayne Mouse Paw\n");
    return 0;
}

static void __exit wayne_mouse_exit(void)
{
    input_unregister_device(wayne_mouse_dev);
    pr_info("WAYNE MOUSE 16.0: mouse removido\n");
}

module_init(wayne_mouse_init);
module_exit(wayne_mouse_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 16 - WAYNE MOUSE - move + clica");
