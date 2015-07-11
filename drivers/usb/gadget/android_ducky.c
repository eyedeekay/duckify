#include "f_hid.h"
#include "f_hid_android_keyboard.c"
#include "f_hid_android_mouse.c"

static int hid_function_init(struct android_usb_function *f, struct usb_composite_dev *cdev)
{
	return ghid_setup(cdev->gadget, 2);
}

static void hid_function_cleanup(struct android_usb_function *f)
{
	ghid_cleanup();
}

static int hid_function_bind_config(struct android_usb_function *f, struct usb_configuration *c)
{
	int ret;
	printk(KERN_INFO "hid keyboard\n");
	ret = hidg_bind_config(c, &ghid_device_android_keyboard, 0);
	if (ret) {
		pr_info("%s: hid_function_bind_config keyboard failed: %d\n", __func__, ret);
		return ret;
	}
	printk(KERN_INFO "hid mouse\n");
	ret = hidg_bind_config(c, &ghid_device_android_mouse, 1);
	if (ret) {
		pr_info("%s: hid_function_bind_config mouse failed: %d\n", __func__, ret);
		return ret;
	}
	return 0;
}

static struct android_usb_function hid_function = {
	.name		= "hid",
	.init		= hid_function_init,
	.cleanup	= hid_function_cleanup,
	.bind_config	= hid_function_bind_config,
};