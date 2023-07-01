#include "processExamples.h"
#include <stdint.h>
#include <stdio.h>
#include "processManager.h"
#include "kbd.h"

int idle(void*) {
    for (;;) {
        sti();
        hlt();
        cli();
    }
    return 0;
}

int proc1(void*) {
    for (int32_t i = 0; i < 2; i++) {
        printf("[temps = %03d] processus %s pid = %i\n", get_system_time(),
               my_name(), getpid());
        sleep(2);  // 2 segundos - 100 interrups
    }
    return 0;
}

int proc2(void*) {
    for (;;) {
        printf("[temps = %03d] processus %s pid = %i\n", get_system_time(),
               my_name(), getpid());
        sleep(3);
    }
    return 0;
}

int proc3(void*)
{
    for (;;)
    {
        printf("[temps = %03d] processus %s pid = %i\n", get_system_time(),
               my_name(), getpid());
        sleep(5);
    }
    return 0;
}

int test_keyboard(void*)
{
    for (int32_t i = 0; i < 2; i++)
    {
        printf("[temps = %i] processus %s pid = %i\n", get_system_time(),
               my_name(), getpid());
        
        printf("Write something:");
        char buff[25];
        cons_read(buff, 25);
        cons_write(buff, 25);
    }
    return 0;
}