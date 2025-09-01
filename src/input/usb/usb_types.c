#include "usb_types.h"

// USB speed names for debugging
const char* usb_speed_names[USB_SPEED_COUNT] = {
    "Unknown",
    "Low Speed (1.5 Mbps)",
    "Full Speed (12 Mbps)",
    "High Speed (480 Mbps)",
    "Super Speed (5 Gbps)",
    "Super Speed+ (10 Gbps)"
};

// USB class names for debugging
const char* usb_class_names[] = {
    "Use Interface Descriptors",    // 0x00
    "Audio",                        // 0x01
    "Communications and CDC Control", // 0x02
    "HID",                          // 0x03
    "Reserved",                     // 0x04
    "Physical",                     // 0x05
    "Image",                        // 0x06
    "Printer",                      // 0x07
    "Mass Storage",                 // 0x08
    "Hub",                          // 0x09
    "CDC Data",                     // 0x0A
    "Smart Card",                   // 0x0B
    "Reserved",                     // 0x0C
    "Content Security",             // 0x0D
    "Video",                        // 0x0E
    "Personal Healthcare",          // 0x0F
    "Audio/Video Devices",          // 0x10
    "Billboard Device",             // 0x11
    "USB Type-C Bridge",            // 0x12
    "Reserved"                      // 0x13-0xDC
};

const char* usb_get_speed_name(usb_speed_t speed) {
    if (speed >= USB_SPEED_COUNT) {
        return usb_speed_names[USB_SPEED_UNKNOWN];
    }
    return usb_speed_names[speed];
}

const char* usb_get_class_name(uint8_t class_code) {
    if (class_code <= 0x12) {
        return usb_class_names[class_code];
    } else if (class_code == 0xDC) {
        return "Diagnostic Device";
    } else if (class_code == 0xE0) {
        return "Wireless Controller";
    } else if (class_code == 0xEF) {
        return "Miscellaneous";
    } else if (class_code == 0xFE) {
        return "Application Specific";
    } else if (class_code == 0xFF) {
        return "Vendor Specific";
    } else {
        return "Reserved";
    }
}

int usb_is_valid_descriptor_type(uint8_t type) {
    switch (type) {
        case USB_DESC_DEVICE:
        case USB_DESC_CONFIGURATION:
        case USB_DESC_STRING:
        case USB_DESC_INTERFACE:
        case USB_DESC_ENDPOINT:
        case USB_DESC_DEVICE_QUALIFIER:
        case USB_DESC_OTHER_SPEED_CONFIG:
        case USB_DESC_INTERFACE_POWER:
        case USB_DESC_OTG:
        case USB_DESC_DEBUG:
        case USB_DESC_INTERFACE_ASSOCIATION:
        case USB_DESC_BOS:
        case USB_DESC_DEVICE_CAPABILITY:
        case USB_DESC_SUPERSPEED_HUB:
        case USB_DESC_SUPERSPEED_ENDPOINT_COMPANION:
            return 1;
        default:
            return 0;
    }
}

uint16_t usb_calculate_max_packet_size(usb_speed_t speed, usb_endpoint_type_t type) {
    switch (speed) {
        case USB_SPEED_LOW:
            if (type == USB_ENDPOINT_CONTROL || type == USB_ENDPOINT_INTERRUPT) {
                return 8;
            }
            return 0; // Low speed doesn't support bulk or isochronous

        case USB_SPEED_FULL:
            switch (type) {
                case USB_ENDPOINT_CONTROL:
                    return 64;
                case USB_ENDPOINT_ISOCHRONOUS:
                    return 1023;
                case USB_ENDPOINT_BULK:
                    return 64;
                case USB_ENDPOINT_INTERRUPT:
                    return 64;
            }
            break;

        case USB_SPEED_HIGH:
            switch (type) {
                case USB_ENDPOINT_CONTROL:
                    return 64;
                case USB_ENDPOINT_ISOCHRONOUS:
                    return 1024;
                case USB_ENDPOINT_BULK:
                    return 512;
                case USB_ENDPOINT_INTERRUPT:
                    return 1024;
            }
            break;

        case USB_SPEED_SUPER:
        case USB_SPEED_SUPER_PLUS:
            return 1024; // SuperSpeed uses 1024 bytes for all endpoint types

        default:
            return 0;
    }
    return 0;
}

int usb_validate_device_descriptor(const usb_device_descriptor_t *desc) {
    if (!desc) return 0;

    if (desc->bLength != sizeof(usb_device_descriptor_t)) return 0;
    if (desc->bDescriptorType != USB_DESC_DEVICE) return 0;
    if (desc->bNumConfigurations == 0) return 0;

    return 1;
}

int usb_validate_config_descriptor(const usb_config_descriptor_t *desc) {
    if (!desc) return 0;

    if (desc->bLength != sizeof(usb_config_descriptor_t)) return 0;
    if (desc->bDescriptorType != USB_DESC_CONFIGURATION) return 0;
    if (desc->bNumInterfaces == 0) return 0;
    if (desc->wTotalLength < sizeof(usb_config_descriptor_t)) return 0;

    return 1;
}
