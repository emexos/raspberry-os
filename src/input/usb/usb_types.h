#ifndef USB_TYPES_H
#define USB_TYPES_H

// USB speeds
typedef enum {
    USB_SPEED_UNKNOWN = 0,
    USB_SPEED_LOW,        // 1.5 Mbps
    USB_SPEED_FULL,       // 12 Mbps
    USB_SPEED_HIGH,       // 480 Mbps
    USB_SPEED_SUPER,      // 5 Gbps
    USB_SPEED_SUPER_PLUS, // 10 Gbps
    USB_SPEED_COUNT
} usb_speed_t;

// USB descriptor types
#define USB_DESC_DEVICE                    0x01
#define USB_DESC_CONFIGURATION             0x02
#define USB_DESC_STRING                    0x03
#define USB_DESC_INTERFACE                 0x04
#define USB_DESC_ENDPOINT                  0x05
#define USB_DESC_DEVICE_QUALIFIER          0x06
#define USB_DESC_OTHER_SPEED_CONFIG        0x07
#define USB_DESC_INTERFACE_POWER           0x08
#define USB_DESC_OTG                       0x09
#define USB_DESC_DEBUG                     0x0A
#define USB_DESC_INTERFACE_ASSOCIATION     0x0B
#define USB_DESC_BOS                       0x0F
#define USB_DESC_DEVICE_CAPABILITY         0x10
#define USB_DESC_SUPERSPEED_HUB            0x2A
#define USB_DESC_SUPERSPEED_ENDPOINT_COMPANION 0x30

// USB class codes
#define USB_CLASS_USE_INTERFACE_DESC       0x00
#define USB_CLASS_AUDIO                    0x01
#define USB_CLASS_COMM                     0x02
#define USB_CLASS_HID                      0x03
#define USB_CLASS_PHYSICAL                 0x05
#define USB_CLASS_IMAGE                    0x06
#define USB_CLASS_PRINTER                  0x07
#define USB_CLASS_MASS_STORAGE             0x08
#define USB_CLASS_HUB                      0x09
#define USB_CLASS_CDC_DATA                 0x0A
#define USB_CLASS_SMART_CARD               0x0B
#define USB_CLASS_CONTENT_SECURITY         0x0D
#define USB_CLASS_VIDEO                    0x0E
#define USB_CLASS_PERSONAL_HEALTHCARE      0x0F
#define USB_CLASS_DIAGNOSTIC               0xDC
#define USB_CLASS_WIRELESS                 0xE0
#define USB_CLASS_MISCELLANEOUS            0xEF
#define USB_CLASS_APP_SPECIFIC             0xFE
#define USB_CLASS_VENDOR_SPECIFIC          0xFF

// USB endpoint types
typedef enum {
    USB_ENDPOINT_CONTROL = 0,
    USB_ENDPOINT_ISOCHRONOUS = 1,
    USB_ENDPOINT_BULK = 2,
    USB_ENDPOINT_INTERRUPT = 3
} usb_endpoint_type_t;

// USB device states
typedef enum {
    USB_DEVICE_STATE_DETACHED = 0,
    USB_DEVICE_STATE_ATTACHED,
    USB_DEVICE_STATE_POWERED,
    USB_DEVICE_STATE_DEFAULT,
    USB_DEVICE_STATE_ADDRESS,
    USB_DEVICE_STATE_CONFIGURED,
    USB_DEVICE_STATE_SUSPENDED
} usb_device_state_t;

// USB request types
#define USB_REQ_TYPE_STANDARD              0x00
#define USB_REQ_TYPE_CLASS                 0x20
#define USB_REQ_TYPE_VENDOR                0x40
#define USB_REQ_TYPE_RESERVED              0x60

// USB request recipients
#define USB_REQ_RECIPIENT_DEVICE           0x00
#define USB_REQ_RECIPIENT_INTERFACE        0x01
#define USB_REQ_RECIPIENT_ENDPOINT         0x02
#define USB_REQ_RECIPIENT_OTHER            0x03

// Standard USB requests
#define USB_REQ_GET_STATUS                 0x00
#define USB_REQ_CLEAR_FEATURE              0x01
#define USB_REQ_SET_FEATURE                0x03
#define USB_REQ_SET_ADDRESS                0x05
#define USB_REQ_GET_DESCRIPTOR             0x06
#define USB_REQ_SET_DESCRIPTOR             0x07
#define USB_REQ_GET_CONFIGURATION          0x08
#define USB_REQ_SET_CONFIGURATION          0x09
#define USB_REQ_GET_INTERFACE              0x0A
#define USB_REQ_SET_INTERFACE              0x0B
#define USB_REQ_SYNCH_FRAME                0x0C

// USB device descriptor structure
typedef struct __attribute__((packed)) {
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned short bcdUSB;
    unsigned char  bDeviceClass;
    unsigned char  bDeviceSubClass;
    unsigned char  bDeviceProtocol;
    unsigned char  bMaxPacketSize0;
    unsigned short idVendor;
    unsigned short idProduct;
    unsigned short bcdDevice;
    unsigned char  iManufacturer;
    unsigned char  iProduct;
    unsigned char  iSerialNumber;
    unsigned char  bNumConfigurations;
} usb_device_descriptor_t;

// USB configuration descriptor structure
typedef struct __attribute__((packed)) {
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned short wTotalLength;
    unsigned char  bNumInterfaces;
    unsigned char  bConfigurationValue;
    unsigned char  iConfiguration;
    unsigned char  bmAttributes;
    unsigned char  bMaxPower;
} usb_config_descriptor_t;

// USB interface descriptor structure
typedef struct __attribute__((packed)) {
    unsigned char bLength;
    unsigned char bDescriptorType;
    unsigned char bInterfaceNumber;
    unsigned char bAlternateSetting;
    unsigned char bNumEndpoints;
    unsigned char bInterfaceClass;
    unsigned char bInterfaceSubClass;
    unsigned char bInterfaceProtocol;
    unsigned char iInterface;
} usb_interface_descriptor_t;

// USB endpoint descriptor structure
typedef struct __attribute__((packed)) {
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned char  bEndpointAddress;
    unsigned char  bmAttributes;
    unsigned short wMaxPacketSize;
    unsigned char  bInterval;
} usb_endpoint_descriptor_t;

// USB setup packet structure
typedef struct __attribute__((packed)) {
    unsigned char  bmRequestType;
    unsigned char  bRequest;
    unsigned short wValue;
    unsigned short wIndex;
    unsigned short wLength;
} usb_setup_packet_t;

// USB device structure
typedef struct {
    unsigned char address;
    usb_speed_t speed;
    usb_device_state_t state;
    usb_device_descriptor_t device_desc;
    usb_config_descriptor_t *config_desc;
    unsigned char current_config;
    unsigned short max_packet_size_ep0;
} usb_device_t;

// Function declarations
const char* usb_get_speed_name(usb_speed_t speed);
const char* usb_get_class_name(unsigned char class_code);
int usb_is_valid_descriptor_type(unsigned char type);
unsigned short usb_calculate_max_packet_size(usb_speed_t speed, usb_endpoint_type_t type);
int usb_validate_device_descriptor(const usb_device_descriptor_t *desc);
int usb_validate_config_descriptor(const usb_config_descriptor_t *desc);

#endif
