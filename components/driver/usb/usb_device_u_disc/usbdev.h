#ifndef _USB_DEV_H
#define _USB_DEV_H

#include <stdint.h>
enum usb_action_t{
    USB_ACTION_OUT,
    USB_ACTION_IN,
    USB_ACTION_DATA_TRANSFER,
    USB_ACTION_IDLE,
    USB_ACTION_MAX,
};

void usbdev_set_ep0_retval(uint8_t *ptr, int size);

void usbdev_device_reset(void);

void usbdev_ep0_handler (void);

void usbdev_init(void);

void usb_user_notify(enum usb_action_t flag);
#endif //_USB_DEV_H

