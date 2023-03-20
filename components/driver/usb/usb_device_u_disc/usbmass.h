#ifndef _USB_MASS_H
#define _USB_MASS_H

#include <stdint.h>

#include "usb.h"

#define USBMASS_OUT_EP          1
#define USBMASS_IN_EP           2

void usbmass_event_handler(void);

int get_usbmass_dev_desc(USBREQPACKET *dreq, uint8_t **ptr, int *size);

int get_usbmass_config_desc(USBREQPACKET *dreq, uint8_t * * ptr, int *size);

int get_usbmass_string_desc(USBREQPACKET *dreq, uint8_t * * ptr, int *size);

int get_usbmass_configuration(USBREQPACKET *dreq, uint8_t * * ptr, int *size);

int set_usbmass_interface(USBREQPACKET *dreq);

void usbmass_class_req_dec(USBREQPACKET *dreq);

void usbmass_device_reset(void);

void usbmass_var_init(void);

void SetEPTxStatus(uint8_t bEpNum, uint16_t wState);
void SetEPRxStatus(uint8_t bEpNum, uint16_t wState);
uint16_t GetEPRxStatus(uint8_t bEpNum);
void SetEPTxCount(uint8_t bEpNum, uint16_t wCount);

#endif //_USB_MASS_H

