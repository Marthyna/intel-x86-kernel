#include <stdio.h>

#include "processExamples.h"
#include "processManager.h"
#include "shared_memory.h"
#include "kbd.h"
#include "tests/test0.h"
#include "tests/test1.h"
#include "tests/test2.h"
#include "tests/test3.h"
#include "tests/test4.h"
#include "tests/test5.h"
#include "tests/test6.h"
#include "tests/test7.h"
#include "tests/test8.h"
#include "tests/test10.h"
#include "tests/test12.h"
#include "tests/test14.h"
#include "tests/test15.h"
#include "tests/test17.h"

void kernel_start(void)
{
	clock_init();
	keyboard_config();
	shm_config();
	clear_screen();

	start(idle, 1024, 1, "idle",  NULL);

	// TEST USER -> KERNEL
	start(test0, 512, 128, "test0", NULL);
    start(test1, 512, 128, "test1", NULL);
    start(test2, 512, 128, "test2", NULL);
    start(test3, 512, 128, "test3", NULL);	
    start(test4, 512, 128, "test4", NULL);	
    start(test5, 512, 128, "test5", NULL);	
    start(test6, 512, 128, "test6", NULL);	
    start(test7, 512, 128, "test7", NULL);
    start(test8, 512, 128, "test8", NULL);
    start(test10, 512, 128, "test10", NULL);
    start(test12, 512, 128, "test12", NULL);
    start(test14, 512, 128, "test14", NULL);
    start(test15, 512, 128, "test15", NULL);
    start(test17, 512, 128, "test17", NULL);
    printf("Continuing test 7 ----------------------------------------\n");
    
	idle(NULL);

	while (1)
		hlt();

	return;
}