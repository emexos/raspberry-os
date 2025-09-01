#include "../../include/io.h"
#include "../../include/pcie.h"
#include "xhci.h"

// NULL definition falls nicht im Header
#ifndef NULL
#define NULL ((void*)0)
#endif

// XHCI register offsets
#define XHCI_CAP_CAPLENGTH      0x00
#define XHCI_CAP_HCIVERSION     0x02
#define XHCI_CAP_HCSPARAMS1     0x04
#define XHCI_CAP_HCSPARAMS2     0x08
#define XHCI_CAP_HCSPARAMS3     0x0C
#define XHCI_CAP_HCCPARAMS1     0x10
#define XHCI_CAP_DBOFF          0x14
#define XHCI_CAP_RTSOFF         0x18

// Operational registers
#define XHCI_OP_USBCMD          0x00
#define XHCI_OP_USBSTS          0x04
#define XHCI_OP_PAGESIZE        0x08
#define XHCI_OP_DNCTRL          0x14
#define XHCI_OP_CRCR            0x18
#define XHCI_OP_DCBAAP          0x30
#define XHCI_OP_CONFIG          0x38

// USBCMD register bits
#define XHCI_CMD_RUN            (1 << 0)
#define XHCI_CMD_RESET          (1 << 1)
#define XHCI_CMD_INTE           (1 << 2)
#define XHCI_CMD_HSEE           (1 << 3)

// USBSTS register bits
#define XHCI_STS_HCH            (1 << 0)
#define XHCI_STS_HSE            (1 << 2)
#define XHCI_STS_EINT           (1 << 3)
#define XHCI_STS_PCD            (1 << 4)
#define XHCI_STS_CNR            (1 << 11)
#define XHCI_STS_HCE            (1 << 12)

static xhci_controller_t xhci_controller;
static int xhci_initialized = 0;

static unsigned int xhci_read_cap32(unsigned int offset) {
    return mmio_read((long)(xhci_controller.cap_base + offset));
}

static unsigned int xhci_read_op32(unsigned int offset) {
    return mmio_read((long)(xhci_controller.op_base + offset));
}

static void xhci_write_op32(unsigned int offset, unsigned int value) {
    mmio_write((long)(xhci_controller.op_base + offset), value);
}

static unsigned long long xhci_read_op64(unsigned int offset) {
    unsigned int low = xhci_read_op32(offset);
    unsigned int high = xhci_read_op32(offset + 4);
    return ((unsigned long long)high << 32) | low;
}

static void xhci_write_op64(unsigned int offset, unsigned long long value) {
    xhci_write_op32(offset, (unsigned int)(value & 0xFFFFFFFF));
    xhci_write_op32(offset + 4, (unsigned int)(value >> 32));
}

static int xhci_wait_for_ready(void) {
    int timeout = 1000;

    while (timeout-- > 0) {
        unsigned int status = xhci_read_op32(XHCI_OP_USBSTS);
        if (!(status & XHCI_STS_CNR)) {
            return 1;  // Controller is ready
        }

        // Simple delay
        for (volatile int i = 0; i < 1000; i++);
    }

    return 0;  // Timeout
}

static int xhci_reset_controller(void) {
    // Set halt bit
    unsigned int cmd = xhci_read_op32(XHCI_OP_USBCMD);
    cmd &= ~XHCI_CMD_RUN;
    xhci_write_op32(XHCI_OP_USBCMD, cmd);

    // Wait for halt
    int timeout = 1000;
    while (timeout-- > 0) {
        unsigned int status = xhci_read_op32(XHCI_OP_USBSTS);
        if (status & XHCI_STS_HCH) {
            break;
        }
        for (volatile int i = 0; i < 1000; i++);
    }

    if (timeout <= 0) {
        return 0;  // Failed to halt
    }

    // Reset controller
    cmd = xhci_read_op32(XHCI_OP_USBCMD);
    cmd |= XHCI_CMD_RESET;
    xhci_write_op32(XHCI_OP_USBCMD, cmd);

    // Wait for reset completion
    timeout = 1000;
    while (timeout-- > 0) {
        cmd = xhci_read_op32(XHCI_OP_USBCMD);
        if (!(cmd & XHCI_CMD_RESET)) {
            break;
        }
        for (volatile int i = 0; i < 1000; i++);
    }

    if (timeout <= 0) {
        return 0;  // Reset timeout
    }

    return xhci_wait_for_ready();
}

static void xhci_setup_scratchpad_buffers(void) {
    unsigned int hcsparams2 = xhci_read_cap32(XHCI_CAP_HCSPARAMS2);
    unsigned int max_scratchpad_bufs = ((hcsparams2 & 0x03E00000) >> 21) |
                                   (((hcsparams2 & 0xF8000000) >> 27) << 5);

    xhci_controller.max_scratchpad_bufs = max_scratchpad_bufs;

    // For now, we don't allocate scratchpad buffers as we don't have a proper memory allocator
    // This would be needed for a full implementation
}

int xhci_init(void) {
    if (xhci_initialized) {
        return 1;
    }

    // Find XHCI controller via PCIe
    pcie_device_t *xhci_device = pcie_find_device_by_class(
        PCIE_CLASS_SERIAL_BUS << 8, PCIE_SUBCLASS_USB);

    if (!xhci_device) {
        return 0;  // No XHCI controller found
    }

    // Check if it's actually XHCI (prog_if should be 0x30)
    if (xhci_device->prog_if != PCIE_PROG_IF_XHCI) {
        return 0;  // Not XHCI
    }

    // Map BAR0 (memory mapped registers)
    unsigned int bar0 = xhci_device->bars[0];
    if (!(bar0 & 0x1)) {  // Memory BAR
        xhci_controller.mmio_base = (void *)(unsigned long)(bar0 & ~0xF);
    } else {
        return 0;  // We need memory mapped I/O
    }

    // Enable bus master and memory space
    unsigned int cmd_reg = pcie_device_read_config32(xhci_device, 0x04);
    cmd_reg |= 0x06;  // Bus Master + Memory Space Enable
    pcie_device_write_config32(xhci_device, 0x04, cmd_reg);

    // Setup capability and operational register bases
    xhci_controller.cap_base = xhci_controller.mmio_base;
    unsigned char cap_length = *(unsigned char*)xhci_controller.cap_base;
    xhci_controller.op_base = (unsigned char*)xhci_controller.cap_base + cap_length;

    // Read capability parameters
    unsigned int hcsparams1 = xhci_read_cap32(XHCI_CAP_HCSPARAMS1);
    xhci_controller.max_slots = hcsparams1 & 0xFF;
    xhci_controller.max_interrupters = (hcsparams1 >> 8) & 0x7FF;
    xhci_controller.max_ports = (hcsparams1 >> 24) & 0xFF;

    unsigned int hcsparams2 = xhci_read_cap32(XHCI_CAP_HCSPARAMS2);
    xhci_controller.max_scratchpad_bufs = ((hcsparams2 & 0x03E00000) >> 21) |
                                         (((hcsparams2 & 0xF8000000) >> 27) << 5);

    unsigned int hccparams1 = xhci_read_cap32(XHCI_CAP_HCCPARAMS1);
    xhci_controller.context_size = (hccparams1 & 0x04) ? 64 : 32;

    // Reset the controller
    if (!xhci_reset_controller()) {
        return 0;
    }

    // Setup scratchpad buffers (minimal implementation)
    xhci_setup_scratchpad_buffers();

    // Set max device slots
    unsigned int config = xhci_read_op32(XHCI_OP_CONFIG);
    config &= ~0xFF;
    config |= xhci_controller.max_slots;
    xhci_write_op32(XHCI_OP_CONFIG, config);

    xhci_initialized = 1;
    return 1;
}

int xhci_start(void) {
    if (!xhci_initialized) {
        return 0;
    }

    // Enable interrupts and start the controller
    unsigned int cmd = xhci_read_op32(XHCI_OP_USBCMD);
    cmd |= XHCI_CMD_RUN | XHCI_CMD_INTE;
    xhci_write_op32(XHCI_OP_USBCMD, cmd);

    // Wait for controller to start
    int timeout = 1000;
    while (timeout-- > 0) {
        unsigned int status = xhci_read_op32(XHCI_OP_USBSTS);
        if (!(status & XHCI_STS_HCH)) {
            return 1;  // Controller is running
        }
        for (volatile int i = 0; i < 1000; i++);
    }

    return 0;  // Timeout
}

int xhci_stop(void) {
    if (!xhci_initialized) {
        return 0;
    }

    // Stop the controller
    unsigned int cmd = xhci_read_op32(XHCI_OP_USBCMD);
    cmd &= ~XHCI_CMD_RUN;
    xhci_write_op32(XHCI_OP_USBCMD, cmd);

    // Wait for halt
    int timeout = 1000;
    while (timeout-- > 0) {
        unsigned int status = xhci_read_op32(XHCI_OP_USBSTS);
        if (status & XHCI_STS_HCH) {
            return 1;  // Controller halted
        }
        for (volatile int i = 0; i < 1000; i++);
    }

    return 0;  // Timeout
}

xhci_controller_t* xhci_get_controller(void) {
    if (!xhci_initialized) {
        return NULL;
    }
    return &xhci_controller;
}

unsigned int xhci_get_port_count(void) {
    return xhci_controller.max_ports;
}

int xhci_is_initialized(void) {
    return xhci_initialized;
}
