#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include "segment.h"
#include <stdint.h>
#include "string.h"
#include "processor_structs.h" //to get idt
#include "stdbool.h"
#include "cpu.h"

// Privilege levels.
#define PL_KERNEL 0
#define PL_USER   3

// https://wiki.osdev.org/8259_PIC
// Definitions for the I/O command and data ports of the 8259 Programmable Interrupt Controller (PIC)
#define MASTER_PIC_CMD  0x20    /* IO command port for master PIC */
#define SLAVE_PIC_CMD   0xA0    /* IO command port for slave PIC */
#define MASTER_PIC_DATA 0x21    /* IO data port for master PIC */
#define SLAVE_PIC_DATA  0xA1    /* IO data port for slave PIC */

#define PIC_EOI		    0x20		/* End-of-interrupt command code */

struct interrupt_descriptor{
    uint16_t offset_low;    // Lower 16 bits of the base address of the interrupt handler
    uint16_t selector;      // Code segment selector
    uint8_t zero;           // Reserved field, should be 0
    uint8_t type_attr;      // Type and privilege level attributes
    uint16_t offset_high;   // Upper 16 bits of the base address of the interrupt handler
};

void define_interrupt (int num_interrupt, void (*handler)(void), unsigned char protection_level);
void PIC_sendEOI(unsigned char irq);
void mask_IRQ(uint8_t irq, bool mask);

#endif