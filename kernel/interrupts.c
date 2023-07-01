#include "interrupts.h"

/* NUM_NTERRUPT
 * 33 - Dans notre système, le clavier correspond à l'IRQ 1 qui est redirigée sur l'entrée 33 de l'IDT.
 * 32 - Dans le cas de l'horloge, le contrôleur est programmé pour émettre l'interruption 32.
 * 49 - 49 pour les appels au superviseur (sys_call)*/

void define_interrupt (int num_interrupt, void (*handler)(void), unsigned char protection_level){
    
    struct interrupt_descriptor idt_values;

    idt_values.offset_low = (uint32_t)handler & 0x0000ffff;
    idt_values.selector = KERNEL_CS;
    idt_values.zero = 0;
    idt_values.type_attr = 0x80 | ((protection_level & 0b11) << 5) | 0x0E;
    idt_values.offset_high = ((uint32_t)handler & 0xffff0000) >> 16;

    idt[num_interrupt] = *((uint64_t *)&idt_values);
}

// Function to send an End of Interrupt (EOI) signal to the PIC
// Takes an IRQ (Interrupt Request) number as input
void PIC_sendEOI(unsigned char irq){
	if(irq >= 8)
		outb(PIC_EOI, SLAVE_PIC_CMD); 
 
	outb(PIC_EOI, MASTER_PIC_CMD);
}


// Function to mask (disable) a specific IRQ
// Takes an IRQ number as input
void mask_IRQ(uint8_t irq, bool mask){
    // Determine the port to be used based on the IRQ number
    uint16_t port = irq < 8 ? MASTER_PIC_DATA : SLAVE_PIC_DATA;

    // If the IRQ is from the slave PIC (IRQs 8-15), adjust the IRQ number
    irq =  irq < 8 ? irq             : irq - 8;

    // disable or enable the specified IRQ by setting the corresponding bit to 0 in the PIC data port
    const uint8_t current_mask = inb(port);
    outb(mask ? current_mask | (1 << irq) : current_mask & ~(1 << irq), port);
}