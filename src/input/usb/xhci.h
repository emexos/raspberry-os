#ifndef XHCI_H
#define XHCI_H

// NULL definition
#ifndef NULL
#define NULL ((void*)0)
#endif

// XHCI controller structure
typedef struct {
    void *mmio_base;          // Base address of MMIO registers
    void *cap_base;           // Capability registers base
    void *op_base;            // Operational registers base
    void *runtime_base;       // Runtime registers base
    void *doorbell_base;      // Doorbell registers base

    unsigned int max_slots;       // Maximum device slots
    unsigned int max_interrupters; // Maximum interrupters
    unsigned int max_ports;       // Maximum root hub ports
    unsigned int max_scratchpad_bufs; // Maximum scratchpad buffers
    unsigned int context_size;    // Context size (32 or 64 bytes)

    // Device context base address array (would need proper allocation)
    unsigned long long *dcbaap;

    // Command and event ring pointers (would need proper allocation)
    void *command_ring;
    void *event_ring;
} xhci_controller_t;

// Function declarations
int xhci_init(void);
int xhci_start(void);
int xhci_stop(void);
xhci_controller_t* xhci_get_controller(void);
unsigned int xhci_get_port_count(void);
int xhci_is_initialized(void);

#endif
