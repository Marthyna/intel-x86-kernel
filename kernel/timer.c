#include "timer.h"


Time time = {
    .hours = 0,
    .minutes = 0,
    .seconds = 0,
    .ms = 0,
    .total_seconds = 0,
    .nb_interrupt = 0};

void display_top_right(const char *s) {
    Cursor previous_cursor = get_cursor();
    place_cursor(0, MAX_COL - 10);
    console_putbytes(s, 8);
    place_cursor(previous_cursor.lig, previous_cursor.col);
}

void adjust_frequency(void) {
    /* send the 8-bit command 0x34 to the port 0x43 thanks to the outb
     processor: this command indicates to the clock that it is going to
     send it the frequency setting value in the form of two in the form
     of two values of 8 bits each which will be will be sent on the data port
    */
    outb(0x34, 0x43);

    /* we send the 8 least significant bits of the frequency setting value
    on the data port 0x40 : this can be done simply by
    outb((QUARTZ / CLOCKFREQ) & 0xFF, 0x40) where QUARTZ is 0x1234DD and
    CLOCKFREQ is 50
    */
    outb((QUARTZ / CLOCKFREQ) & 0xFF, 0x40);

    /* we then send the 8 most significant bits of the the set value on the same port 0x40 */
    outb((QUARTZ / CLOCKFREQ) >> 8, 0x40);
}

void clock_init()
{
    adjust_frequency();

    define_interrupt(32, clock_interrupt_handler, PL_KERNEL);

    mask_IRQ(0, false);
}

void increment_time(void) {
    time.nb_interrupt++;
    if (time.ms < 1000 - (1000 / CLOCKFREQ)) {
        time.ms += (1000 / CLOCKFREQ);
    } else {
        time.ms = 0;
        time.total_seconds++;
        time.seconds = time.total_seconds % 60;
        time.minutes = (time.total_seconds / 60) % 60;
        time.hours = (time.total_seconds / (60 * 60));
    }
}

// save cursor position and place cursor at the end of the function
void clock_interrupt(void)
{
    PIC_sendEOI(32);
    char s[9] = {0};
    increment_time();
    if (time.ms < 20) {
        sprintf(s, "%02d:%02d:%02d", 0, time.minutes, time.seconds);
        display_top_right(s);
    }
    scheduling();
}

void mask_int_requests(uint32_t num_IRQ, bool mask) {
    /* we must first read the current value of the mask on data port 0x21
     with the the inb function */
    uint8_t irq = inb(IRQZ);

    /* the recovered byte is in fact an array of such that the value of
     the N bit describes the state of the IRQN : 1
     if the IRQ is masked, 0 if it is authorized: it is thus necessary to
     force the value of bit N to the desired value desired value (without
     affecting the values of the the other bits) and send this mask on the
     data port 0x21 thanks to the outb function
    */
    irq = mask ? irq | 1 << num_IRQ : irq & ~(1 << num_IRQ);
    outb(irq, IRQZ);
}

/*Returns in *quartz the frequency of the quartz of the system and in
 *tics the number of oscillations of the quartz between each interruption.*/
void clock_settings(unsigned long *quartz, unsigned long *tics) {
    *quartz = QUARTZ;
    *tics = QUARTZ / CLOCKFREQ;
}

/*Returns the number of clock interrupts since the kernel was started.*/
unsigned long current_clock(void) {
    return time.nb_interrupt;
}

int get_system_time() {
    // return the total time since the system started
    return time.total_seconds;
}