#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>

static struct input_dev *wayne_sensor_dev;

static int __init wayne_sensor_init(void)
{
    int err;
    pr_info("WAYNE SENSOR 14.0: iniciando o sensor\n");

    wayne_sensor_dev = input_allocate_device();
    if (!wayne_sensor_dev)
        return -ENOMEM;

    wayne_sensor_dev->name = "Wayne Sensor Paw";
    wayne_sensor_dev->phys = "wayne/sensor0";
    wayne_sensor_dev->id.bustype = BUS_VIRTUAL;
    wayne_sensor_dev->id.vendor = 0x1234;
    wayne_sensor_dev->id.product = 0x0014;
    wayne_sensor_dev->id.version = 0x1400;

    __set_bit(EV_ABS, wayne_sensor_dev->evbit);
    __set_bit(EV_SYN, wayne_sensor_dev->evbit);

    input_set_abs_params(wayne_sensor_dev, ABS_X, -1000, 1000, 0, 0);
    input_set_abs_params(wayne_sensor_dev, ABS_Y, -1000, 1000, 0, 0);
    input_set_abs_params(wayne_sensor_dev, ABS_Z, -1000, 1000, 0, 0);

    err = input_register_device(wayne_sensor_dev);
    if (err) {
        input_free_device(wayne_sensor_dev);
        pr_err("WAYNE SENSOR 14.0: falhou ao registrar\n");
        return err;
    }

    /* Sensor: simula o celular inclinado */
    input_report_abs(wayne_sensor_dev, ABS_X, 100);
    input_report_abs(wayne_sensor_dev, ABS_Y, -200);
    input_report_abs(wayne_sensor_dev, ABS_Z, 900);
    input_sync(wayne_sensor_dev);

    pr_info("WAYNE SENSOR 14.0: sensor enviado - X=100 Y=-200 Z=900 - Wayne Sensor Paw\n");
    return 0;
}

static void __exit wayne_sensor_exit(void)
{
    input_unregister_device(wayne_sensor_dev);
    pr_info("WAYNE SENSOR 14.0: sensor removido\n");
}

module_init(wayne_sensor_init);
module_exit(wayne_sensor_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wayne Lab");
MODULE_DESCRIPTION("Projeto 14 - WAYNE SENSOR - acelerometro");
