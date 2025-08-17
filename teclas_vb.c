#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/uinput.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bizo");
MODULE_DESCRIPTION("Troca V<->B usando input_handler com dispositivo virtual");

static struct input_dev *virtual_dev = NULL;

struct kb_handle {
    struct input_handle ih;
    bool processing;
};

static bool swap_filter(struct input_handle *ih,
                        unsigned int type, unsigned int code, int value)
{
    struct kb_handle *kh = container_of(ih, struct kb_handle, ih);
   
    if (type != EV_KEY || (value != 0 && value != 1))
        return false;
   
    if (kh->processing)
        return false;
   
    if (code == KEY_V || code == KEY_B) {
        unsigned int newcode = (code == KEY_V) ? KEY_B : KEY_V;
       
        kh->processing = true;
       
        if (virtual_dev) {
            input_report_key(virtual_dev, newcode, value);
            input_sync(virtual_dev);
        }
       
        kh->processing = false;
       
        return true;
    }
   
    return false;
}

static int swap_connect(struct input_handler *handler, struct input_dev *dev,
                        const struct input_device_id *id)
{
    struct kb_handle *kh;
   
    if (!test_bit(EV_KEY, dev->evbit))
        return -ENODEV;
   
    if (dev == virtual_dev)
        return -ENODEV;
   
    if (!test_bit(KEY_V, dev->keybit) && !test_bit(KEY_B, dev->keybit))
        return -ENODEV;
   
    kh = kzalloc(sizeof(*kh), GFP_KERNEL);
    if (!kh)
        return -ENOMEM;
   
    kh->ih.dev = dev;
    kh->ih.handler = handler;
    kh->ih.name = "swap_vb_handle";
    kh->processing = false;
   
    if (input_register_handle(&kh->ih)) {
        kfree(kh);
        return -ENODEV;
    }
   
    if (input_open_device(&kh->ih)) {
        input_unregister_handle(&kh->ih);
        kfree(kh);
        return -ENODEV;
    }
   
    pr_info("swap_vb: conectado a %s\n", dev_name(&dev->dev));
    return 0;
}

static void swap_disconnect(struct input_handle *ih)
{
    struct kb_handle *kh = container_of(ih, struct kb_handle, ih);
   
    pr_info("swap_vb: desconectando de %s\n", dev_name(&ih->dev->dev));
    input_close_device(&kh->ih);
    input_unregister_handle(&kh->ih);
    kfree(kh);
}

static const struct input_device_id swap_ids[] = {
    {
        .flags = INPUT_DEVICE_ID_MATCH_EVBIT,
        .evbit = { BIT_MASK(EV_KEY) },
    },
    { },
};

MODULE_DEVICE_TABLE(input, swap_ids);

static struct input_handler swap_handler = {
    .filter     = swap_filter,
    .connect    = swap_connect,
    .disconnect = swap_disconnect,
    .name       = "swap_vb",
    .id_table   = swap_ids,
};

static int create_virtual_device(void)
{
    int ret;
   
    virtual_dev = input_allocate_device();
    if (!virtual_dev) {
        pr_err("swap_vb: falha ao alocar dispositivo virtual\n");
        return -ENOMEM;
    }
   
    virtual_dev->name = "Virtual Keyboard Swapper";
    virtual_dev->id.bustype = BUS_VIRTUAL;
    virtual_dev->id.vendor  = 0x0001;
    virtual_dev->id.product = 0x0001;
    virtual_dev->id.version = 0x0001;
   
    set_bit(EV_KEY, virtual_dev->evbit);
    set_bit(EV_SYN, virtual_dev->evbit);
   
    set_bit(KEY_V, virtual_dev->keybit);
    set_bit(KEY_B, virtual_dev->keybit);
   
    ret = input_register_device(virtual_dev);
    if (ret) {
        pr_err("swap_vb: falha ao registrar dispositivo virtual (%d)\n", ret);
        input_free_device(virtual_dev);
        virtual_dev = NULL;
        return ret;
    }
   
    pr_info("swap_vb: dispositivo virtual criado\n");
    return 0;
}

static void destroy_virtual_device(void)
{
    if (virtual_dev) {
        input_unregister_device(virtual_dev);
        virtual_dev = NULL;
        pr_info("swap_vb: dispositivo virtual destruído\n");
    }
}

static int __init swap_init(void)
{
    int ret;
   
    ret = create_virtual_device();
    if (ret)
        return ret;
   
    /* Registra o handler */
    ret = input_register_handler(&swap_handler);
    if (ret) {
        pr_err("swap_vb: falha ao registrar handler (%d)\n", ret);
        destroy_virtual_device();
        return ret;
    }
   
    pr_info("swap_vb: módulo carregado (trocando V<->B)\n");
    return 0;
}

static void __exit swap_exit(void)
{
    input_unregister_handler(&swap_handler);
    destroy_virtual_device();
    pr_info("swap_vb: módulo descarregado\n");
}

module_init(swap_init);
module_exit(swap_exit);