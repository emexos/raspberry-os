#include "usb_manager.h"
#include "xhci.h"
#include "../../include/pcie.h"

#define MAX_USB_DEVICES 16

static usb_manager_t usb_manager;
static usb_device_t usb_devices[MAX_USB_DEVICES];
static int usb_manager_initialized = 0;

static void usb_manager_reset(void) {
    usb_manager.device_count = 0;
    usb_manager.xhci_controller = NULL;

    // Initialize device array
    for (int i = 0; i < MAX_USB_DEVICES; i++) {
        usb_devices[i].address = 0;
        usb_devices[i].speed = USB_SPEED_UNKNOWN;
        usb_devices[i].state = USB_DEVICE_STATE_DETACHED;
        usb_devices[i].current_config = 0;
        usb_devices[i].max_packet_size_ep0 = 0;
        usb_devices[i].config_desc = NULL;
    }
}

int usb_manager_init(void) {
    if (usb_manager_initialized) {
        return 1;
    }

    usb_manager_reset();

    // Initialize PCIe subsystem first
    if (!pcie_init()) {
        return 0;
    }

    // Initialize XHCI controller
    if (!xhci_init()) {
        return 0;
    }

    usb_manager.xhci_controller = xhci_get_controller();
    if (!usb_manager.xhci_controller) {
        return 0;
    }

    // Start the XHCI controller
    if (!xhci_start()) {
        return 0;
    }

    usb_manager_initialized = 1;
    return 1;
}

int usb_manager_shutdown(void) {
    if (!usb_manager_initialized) {
        return 1;
    }

    // Stop XHCI controller
    xhci_stop();

    usb_manager_reset();
    usb_manager_initialized = 0;

    return 1;
}

int usb_manager_scan_devices(void) {
    if (!usb_manager_initialized || !usb_manager.xhci_controller) {
        return 0;
    }

    // This would normally scan all XHCI ports for connected devices
    // For now, this is a placeholder implementation
    uint32_t port_count = xhci_get_port_count();

    // Reset device count
    usb_manager.device_count = 0;

    // In a full implementation, we would:
    // 1. Check each port status
    // 2. Detect newly connected devices
    // 3. Perform USB enumeration (reset, get descriptor, set address, etc.)
    // 4. Add devices to our device list

    // For now, we just report the number of available ports
    return port_count;
}

usb_device_t* usb_manager_get_device(int index) {
    if (!usb_manager_initialized || index < 0 || index >= usb_manager.device_count) {
        return NULL;
    }

    return &usb_devices[index];
}

int usb_manager_get_device_count(void) {
    if (!usb_manager_initialized) {
        return 0;
    }

    return usb_manager.device_count;
}

usb_device_t* usb_manager_find_device_by_class(uint8_t class_code) {
    if (!usb_manager_initialized) {
        return NULL;
    }

    for (int i = 0; i < usb_manager.device_count; i++) {
        if (usb_devices[i].device_desc.bDeviceClass == class_code) {
            return &usb_devices[i];
        }
    }

    return NULL;
}

usb_device_t* usb_manager_find_device_by_vendor_product(uint16_t vendor_id, uint16_t product_id) {
    if (!usb_manager_initialized) {
        return NULL;
    }

    for (int i = 0; i < usb_manager.device_count; i++) {
        if (usb_devices[i].device_desc.idVendor == vendor_id &&
            usb_devices[i].device_desc.idProduct == product_id) {
            return &usb_devices[i];
        }
    }

    return NULL;
}

int usb_manager_enumerate_device(uint8_t port_number) {
    if (!usb_manager_initialized || usb_manager.device_count >= MAX_USB_DEVICES) {
        return 0;
    }

    // This would perform the full USB enumeration process:
    // 1. Port reset
    // 2. Get device descriptor (first 8 bytes)
    // 3. Set device address
    // 4. Get full device descriptor
    // 5. Get configuration descriptor
    // 6. Set configuration

    // For now, this is a placeholder
    usb_device_t *device = &usb_devices[usb_manager.device_count];

    // Initialize device with default values
    device->address = usb_manager.device_count + 1;
    device->speed = USB_SPEED_HIGH; // Assume high speed for now
    device->state = USB_DEVICE_STATE_CONFIGURED;
    device->max_packet_size_ep0 = 64;
    device->current_config = 1;

    usb_manager.device_count++;

    return 1;
}

int usb_manager_reset_device(usb_device_t *device) {
    if (!usb_manager_initialized || !device) {
        return 0;
    }

    // Reset device state
    device->state = USB_DEVICE_STATE_DEFAULT;
    device->address = 0;
    device->current_config = 0;

    // In a full implementation, this would send reset commands to the device

    return 1;
}

int usb_manager_configure_device(usb_device_t *device, uint8_t config_value) {
    if (!usb_manager_initialized || !device) {
        return 0;
    }

    // In a full implementation, this would:
    // 1. Send SET_CONFIGURATION request
    // 2. Update device state
    // 3. Initialize endpoints

    device->current_config = config_value;
    device->state = USB_DEVICE_STATE_CONFIGURED;

    return 1;
}

usb_manager_t* usb_manager_get_instance(void) {
    if (!usb_manager_initialized) {
        return NULL;
    }

    return &usb_manager;
}

int usb_manager_is_initialized(void) {
    return usb_manager_initialized;
}
