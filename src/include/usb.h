#ifndef USB_H
#define USB_H

// USB system status structure
typedef struct {
    int initialized;
    int xhci_initialized;
    int pcie_device_count;
    int usb_device_count;
    int port_count;
} usb_status_info_t;

// Main USB subsystem functions
int usb_init(void);
int usb_shutdown(void);
void usb_update(void);

// Status and information
void usb_get_status(usb_status_info_t *status);
void usb_print_info(void (*print_func)(const char*));

// Device management
int usb_enumerate_devices(void);

#endif
