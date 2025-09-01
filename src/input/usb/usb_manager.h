#ifndef USB_MANAGER_H
#define USB_MANAGER_H

// NULL definition
#ifndef NULL
#define NULL ((void*)0)
#endif

#include "usb_types.h"
#include "xhci.h"

// USB Manager structure
typedef struct {
    xhci_controller_t *xhci_controller;
    int device_count;
    unsigned int port_count;
} usb_manager_t;

// Function declarations
int usb_manager_init(void);
int usb_manager_shutdown(void);
int usb_manager_scan_devices(void);

// Device management
usb_device_t* usb_manager_get_device(int index);
int usb_manager_get_device_count(void);
usb_device_t* usb_manager_find_device_by_class(unsigned char class_code);
usb_device_t* usb_manager_find_device_by_vendor_product(unsigned short vendor_id, unsigned short product_id);

// Device enumeration and control
int usb_manager_enumerate_device(unsigned char port_number);
int usb_manager_reset_device(usb_device_t *device);
int usb_manager_configure_device(usb_device_t *device, unsigned char config_value);

// Manager instance
usb_manager_t* usb_manager_get_instance(void);
int usb_manager_is_initialized(void);

#endif
