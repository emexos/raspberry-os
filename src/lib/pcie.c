#include "../include/io.h"
#include "../include/pcie.h"

#define PCIE_BASE 0xFD500000  // PCIe controller base on RPi4
#define PCIE_CONFIG_SPACE 0x600000000ULL  // PCIe config space

// PCIe register offsets
#define PCIE_RC_CFG_VENDOR_VENDOR_SPECIFIC_REG1 0x188C
#define PCIE_RC_CFG_PRIV1_ID_VAL3 0x043C
#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LO 0x400C
#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_HI 0x4010
#define PCIE_MISC_RC_BAR1_CONFIG_LO 0x402C
#define PCIE_MISC_RC_BAR2_CONFIG_LO 0x4034
#define PCIE_MISC_RC_BAR2_CONFIG_HI 0x4038
#define PCIE_MISC_RC_BAR3_CONFIG_LO 0x403C
#define PCIE_MISC_MSI_BAR_CONFIG_LO 0x4044
#define PCIE_MISC_MSI_BAR_CONFIG_HI 0x4048
#define PCIE_MISC_MSI_DATA_CONFIG 0x404C
#define PCIE_MISC_PCIE_CTRL 0x4064
#define PCIE_MISC_PCIE_STATUS 0x4068
#define PCIE_MISC_REVISION 0x406C
#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT 0x4070
#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_HI 0x4080
#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LIMIT_HI 0x4084
#define PCIE_MISC_HARD_DEBUG 0x4204
#define PCIE_MSI_INTR2_CLR 0x4508
#define PCIE_MSI_INTR2_MASK_SET 0x4510
#define PCIE_MSI_INTR2_MASK_CLR 0x4514
#define PCIE_EXT_CFG_DATA 0x8000
#define PCIE_EXT_CFG_INDEX 0x9000

// PCIe device information
static pcie_device_t pcie_devices[MAX_PCIE_DEVICES];
static int pcie_device_count = 0;
static int pcie_initialized = 0;

static void pcie_write_reg(unsigned int offset, unsigned int value) {
    mmio_write(PCIE_BASE + offset, value);
}

static unsigned int pcie_read_reg(unsigned int offset) {
    return mmio_read(PCIE_BASE + offset);
}

static void pcie_config_write32(unsigned char bus, unsigned char device, unsigned char function, unsigned char offset, unsigned int value) {
    unsigned int config_addr = (bus << 20) | (device << 15) | (function << 12) | offset;
    pcie_write_reg(PCIE_EXT_CFG_INDEX, config_addr);
    pcie_write_reg(PCIE_EXT_CFG_DATA, value);
}

static unsigned int pcie_config_read32(unsigned char bus, unsigned char device, unsigned char function, unsigned char offset) {
    unsigned int config_addr = (bus << 20) | (device << 15) | (function << 12) | offset;
    pcie_write_reg(PCIE_EXT_CFG_INDEX, config_addr);
    return pcie_read_reg(PCIE_EXT_CFG_DATA);
}

static unsigned short pcie_config_read16(unsigned char bus, unsigned char device, unsigned char function, unsigned char offset) {
    unsigned int data = pcie_config_read32(bus, device, function, offset & ~3);
    return (data >> ((offset & 3) * 8)) & 0xFFFF;
}

static unsigned char pcie_config_read8(unsigned char bus, unsigned char device, unsigned char function, unsigned char offset) {
    unsigned int data = pcie_config_read32(bus, device, function, offset & ~3);
    return (data >> ((offset & 3) * 8)) & 0xFF;
}

static int pcie_setup_bridge(void) {
    // Enable PCIe controller
    pcie_write_reg(PCIE_MISC_PCIE_CTRL, 0x1);

    // Wait for link up
    for (int i = 0; i < 1000; i++) {
        unsigned int status = pcie_read_reg(PCIE_MISC_PCIE_STATUS);
        if (status & 0x1) {  // Link up
            return 1;
        }
        // Simple delay
        for (volatile int j = 0; j < 10000; j++);
    }

    return 0;  // Link failed to come up
}

static void pcie_scan_device(unsigned char bus, unsigned char device, unsigned char function) {
    if (pcie_device_count >= MAX_PCIE_DEVICES) {
        return;
    }

    unsigned short vendor_id = pcie_config_read16(bus, device, function, 0x00);
    if (vendor_id == 0xFFFF) {
        return;  // No device
    }

    pcie_device_t *dev = &pcie_devices[pcie_device_count];
    dev->bus = bus;
    dev->device = device;
    dev->function = function;
    dev->vendor_id = vendor_id;
    dev->device_id = pcie_config_read16(bus, device, function, 0x02);
    dev->class_code = pcie_config_read16(bus, device, function, 0x0A);
    dev->subclass = pcie_config_read8(bus, device, function, 0x0A);
    dev->prog_if = pcie_config_read8(bus, device, function, 0x09);

    // Read BARs
    for (int i = 0; i < 6; i++) {
        dev->bars[i] = pcie_config_read32(bus, device, function, 0x10 + i * 4);
    }

    pcie_device_count++;
}

static void pcie_scan_bus(unsigned char bus) {
    for (unsigned char device = 0; device < 32; device++) {
        for (unsigned char function = 0; function < 8; function++) {
            pcie_scan_device(bus, device, function);

            // Check if this is a multi-function device
            if (function == 0) {
                unsigned char header_type = pcie_config_read8(bus, device, function, 0x0E);
                if (!(header_type & 0x80)) {
                    break;  // Single function device
                }
            }
        }
    }
}

int pcie_init(void) {
    if (pcie_initialized) {
        return 1;
    }

    pcie_device_count = 0;

    // Setup PCIe bridge
    if (!pcie_setup_bridge()) {
        return 0;  // Failed to initialize PCIe
    }

    // Scan for devices
    pcie_scan_bus(0);  // Start with bus 0

    pcie_initialized = 1;
    return 1;
}

int pcie_get_device_count(void) {
    return pcie_device_count;
}

pcie_device_t* pcie_get_device(int index) {
    if (index < 0 || index >= pcie_device_count) {
        return (pcie_device_t*)0;
    }
    return &pcie_devices[index];
}

pcie_device_t* pcie_find_device_by_class(unsigned short class_code, unsigned char subclass) {
    for (int i = 0; i < pcie_device_count; i++) {
        pcie_device_t *dev = &pcie_devices[i];
        if ((dev->class_code >> 8) == (class_code >> 8) && dev->subclass == subclass) {
            return dev;
        }
    }
    return (pcie_device_t*)0;
}

pcie_device_t* pcie_find_device_by_vendor(unsigned short vendor_id, unsigned short device_id) {
    for (int i = 0; i < pcie_device_count; i++) {
        pcie_device_t *dev = &pcie_devices[i];
        if (dev->vendor_id == vendor_id && dev->device_id == device_id) {
            return dev;
        }
    }
    return (pcie_device_t*)0;
}

unsigned int pcie_device_read_config32(pcie_device_t *dev, unsigned char offset) {
    return pcie_config_read32(dev->bus, dev->device, dev->function, offset);
}

void pcie_device_write_config32(pcie_device_t *dev, unsigned char offset, unsigned int value) {
    pcie_config_write32(dev->bus, dev->device, dev->function, offset, value);
}

unsigned short pcie_device_read_config16(pcie_device_t *dev, unsigned char offset) {
    return pcie_config_read16(dev->bus, dev->device, dev->function, offset);
}

unsigned char pcie_device_read_config8(pcie_device_t *dev, unsigned char offset) {
    return pcie_config_read8(dev->bus, dev->device, dev->function, offset);
}
