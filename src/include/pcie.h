#ifndef PCIE_H
#define PCIE_H

#define MAX_PCIE_DEVICES 32

// PCIe device structure
typedef struct {
    unsigned char bus;
    unsigned char device;
    unsigned char function;
    unsigned short vendor_id;
    unsigned short device_id;
    unsigned short class_code;
    unsigned char subclass;
    unsigned char prog_if;
    unsigned int bars[6];
} pcie_device_t;

// PCIe class codes
#define PCIE_CLASS_SERIAL_BUS           0x0C
#define PCIE_SUBCLASS_USB               0x03
#define PCIE_PROG_IF_XHCI               0x30

// Function declarations
int pcie_init(void);
int pcie_get_device_count(void);
pcie_device_t* pcie_get_device(int index);
pcie_device_t* pcie_find_device_by_class(unsigned short class_code, unsigned char subclass);
pcie_device_t* pcie_find_device_by_vendor(unsigned short vendor_id, unsigned short device_id);

// Device configuration access
unsigned int pcie_device_read_config32(pcie_device_t *dev, unsigned char offset);
void pcie_device_write_config32(pcie_device_t *dev, unsigned char offset, unsigned int value);
unsigned short pcie_device_read_config16(pcie_device_t *dev, unsigned char offset);
unsigned char pcie_device_read_config8(pcie_device_t *dev, unsigned char offset);

#endif
