
#include <linux/poll.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/usb/g_hid.h>
#include "f_hid.h"

/* Hacky device list to fix f_hidg_write being called after device destroyed.
   It covers only most common race conditions, there will be rare crashes anyway. */
enum { HACKY_DEVICE_LIST_SIZE = 4 };
static struct f_hidg *hacky_device_list[HACKY_DEVICE_LIST_SIZE];
static void hacky_device_list_add(struct f_hidg *hidg)
{
	int i;
	for (i = 0; i < HACKY_DEVICE_LIST_SIZE; i++) {
		if (!hacky_device_list[i]) {
			hacky_device_list[i] = hidg;
			return;
		}
	}
	pr_err("%s: too many devices, not adding device %p\n", __func__, hidg);
}
static void hacky_device_list_remove(struct f_hidg *hidg)
{
	int i;
	for (i = 0; i < HACKY_DEVICE_LIST_SIZE; i++) {
		if (hacky_device_list[i] == hidg) {
			hacky_device_list[i] = NULL;
			return;
		}
	}
	pr_err("%s: cannot find device %p\n", __func__, hidg);
}
static int hacky_device_list_check(struct f_hidg *hidg)
{
	int i;
	for (i = 0; i < HACKY_DEVICE_LIST_SIZE; i++) {
		if (hacky_device_list[i] == hidg) {
			return 0;
		}
	}
	return 1;
}

static void hacky_device_read_err(struct f_hidg *hidg){
	if (hacky_device_list_check(hidg)) {
		pr_err("%s: trying to read from device %p that was destroyed\n", __func__, hidg);
		return -EIO;
	}
}

static void hacky_device_write_err(struct f_hidg *hidg){
	if (hacky_device_list_check(hidg)) {
		pr_err("%s: trying to write to device %p that was destroyed\n", __func__, hidg);
		return -EIO;
	}
}

static void hacky_device_poll_err(struct f_hidg *hidg){
	if (hacky_device_list_check(hidg)) {
		pr_err("%s: trying to poll device %p that was destroyed\n", __func__, hidg);
		return -EIO;
	}
}

static void hacky_device_list_check_protocol(struct f_hidg *hidg){
	length = min_t(unsigned, length, 1);
	if (hidg->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT)
		((u8 *) req->buf)[0] = 0;	/* Boot protocol */
	else
		((u8 *) req->buf)[0] = 1;	/* Report protocol */
	goto respond;
}

static void hacky_device_list_setup_protocol(struct f_hidg *hidg){
	length = 0;
	if (hidg->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT) {
		if (value == 0)		/* Boot protocol */
			goto respond;
	} else {
		if (value == 1)		/* Report protocol */
			goto respond;
	}
}