#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdbool.h>
#include <stdio.h>

#include "cpu.h"
#include "screen.h"
#include "segment.h"
#include "processManager.h"
#include "interrupts.h"
#include "isr.h"

#define QUARTZ 0x1234DD
#define CLOCKFREQ 50
#define IRQZ 0x21

typedef struct time {
    uint32_t hours;
    uint32_t minutes;
    uint32_t seconds;
    uint16_t ms;
    uint32_t total_seconds;
    uint32_t nb_interrupt;
} Time;

extern Time time;

void clock_isr(void);
void display_top_right(const char *s);
void adjust_frequency(void);
void clock_init();
void tic_PIT(void);
void increment_time(void);
void clock_settings(unsigned long *quartz, unsigned long *ticks);
unsigned long current_clock(void);

int get_system_time();

#endif
