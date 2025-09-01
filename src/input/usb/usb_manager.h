#ifndef USB_MANAGER_H
#define USB_MANAGER_H

#include "usb_types.h"
#include "xhci.h"

// USB Manager structure
typedef struct {
    xhci_controller_t *xhci_controller;
    int device_count;
    uint32_t port_count;
} usb_manager_t;

// Function declarations
int usb_manager_init(void);
int usb_manager_shutdown(void);
int usb_manager_scan_devices(void);

// Device management
usb_device_t* usb_manager_get_device(int index);
int usb_manager_get_device_count(void);
usb_device_t* usb_manager_find_device_by_class(uint8_t class_code);
usb_device_t* usb_manager_find_device_by_vendor_product(uint16_t vendor_id, uint16_t product_id);

// Device enumeration and control
int usb_manager_enumerate_device(uint8_t port_number);
int usb_manager_reset_device(usb_device_t *device);
int usb_manager_configure_device(usb_device_t *device, uint8_t config_value);

// Manager instance
usb_manager_t* usb_manager_get_instance(void);
int usb_manager_is_initialized(void);

#endif
