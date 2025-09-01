#include "usb_manager.h"
#include "usb_types.h"
#include "xhci.h"
#include "../../include/pcie.h"

// NULL definition falls nicht im Header
#ifndef NULL
#define NULL ((void*)0)
#endif

// USB status info structure (missing from your files)
typedef struct {
    int initialized;
    int xhci_initialized;
    int pcie_device_count;
    int usb_device_count;
    unsigned int port_count;
} usb_status_info_t;

// Main USB subsystem initialization
int usb_init(void) {
    // Initialize USB manager (which handles PCIe and XHCI initialization)
    if (!usb_manager_init()) {
        return 0;
    }

    // Perform initial device scan
    int port_count = usb_manager_scan_devices();
    if (port_count < 0) {
        return 0;
    }

    return 1;
}

// USB subsystem shutdown
int usb_shutdown(void) {
    return usb_manager_shutdown();
}

// Get USB subsystem status information
void usb_get_status(usb_status_info_t *status) {
    if (!status) {
        return;
    }

    status->initialized = usb_manager_is_initialized();
    status->xhci_initialized = xhci_is_initialized();
    status->pcie_device_count = pcie_get_device_count();
    status->usb_device_count = usb_manager_get_device_count();
    status->port_count = xhci_is_initialized() ? xhci_get_port_count() : 0;
}

// Print USB system information (for debugging)
void usb_print_info(void (*print_func)(const char*)) {
    if (!print_func) {
        return;
    }

    usb_status_info_t status;
    usb_get_status(&status);

    print_func("=== USB System Information ===\n");

    if (status.initialized) {
        print_func("USB Manager: Initialized\n");
    } else {
        print_func("USB Manager: Not initialized\n");
        return;
    }

    if (status.xhci_initialized) {
        print_func("XHCI Controller: Initialized\n");
    } else {
        print_func("XHCI Controller: Not found/initialized\n");
    }

    // Print basic statistics
    print_func("PCIe devices found: ");
    int pcie_count = status.pcie_device_count;
    if (pcie_count == 0) print_func("0");
    else if (pcie_count == 1) print_func("1");
    else if (pcie_count == 2) print_func("2");
    else if (pcie_count == 3) print_func("3");
    else if (pcie_count == 4) print_func("4");
    else print_func("5+");
    print_func("\n");

    print_func("USB ports available: ");
    int port_count = status.port_count;
    if (port_count == 0) print_func("0");
    else if (port_count == 1) print_func("1");
    else if (port_count == 2) print_func("2");
    else if (port_count == 3) print_func("3");
    else if (port_count == 4) print_func("4");
    else print_func("4+");
    print_func("\n");

    print_func("USB devices connected: ");
    int usb_count = status.usb_device_count;
    if (usb_count == 0) print_func("0");
    else if (usb_count == 1) print_func("1");
    else if (usb_count == 2) print_func("2");
    else print_func("2+");
    print_func("\n");

    // List PCIe devices
    if (status.pcie_device_count > 0) {
        print_func("\n=== PCIe Devices ===\n");
        for (int i = 0; i < status.pcie_device_count && i < 10; i++) {
            pcie_device_t *pcie_dev = pcie_get_device(i);
            if (pcie_dev) {
                print_func("Device ");
                if (i == 0) print_func("0");
                else if (i == 1) print_func("1");
                else if (i == 2) print_func("2");
                else if (i == 3) print_func("3");
                else print_func("N");

                print_func(": ");

                // Check if it's a USB controller
                if ((pcie_dev->class_code >> 8) == PCIE_CLASS_SERIAL_BUS &&
                    pcie_dev->subclass == PCIE_SUBCLASS_USB) {
                    if (pcie_dev->prog_if == PCIE_PROG_IF_XHCI) {
                        print_func("USB 3.0 XHCI Controller");
                    } else {
                        print_func("USB Controller (other)");
                    }
                } else {
                    print_func("Other PCIe device");
                }
                print_func("\n");
            }
        }
    }

    print_func("\n========================\n");
}

// Update USB subsystem (should be called periodically)
void usb_update(void) {
    if (!usb_manager_is_initialized()) {
        return;
    }

    // In a full implementation, this would:
    // 1. Check for device connect/disconnect events
    // 2. Handle pending transfers
    // 3. Update device states
    // 4. Process interrupts

    // For now, this is a placeholder
}

// Simple device enumeration trigger
int usb_enumerate_devices(void) {
    if (!usb_manager_is_initialized()) {
        return 0;
    }

    return usb_manager_scan_devices();
}
