#include "test4.h"
#include "processManager.h"
#include "processExamples.h"

int busy1(void *arg)
{
        (void)arg;
        while (1) {
                int i, j;

                printf(" A");
                for (i=0; i<loop_count1; i++) {
                        test_it();
                        for (j=0; j<loop_count0; j++);
                }
        }
        return 0;
}

/* assume the process to suspend has a priority == 64 */
int busy2(void *arg)
{
        int i;

        for (i = 0; i < 3; i++) {
                int k, j;

                printf(" B");
                for (k=0; k<loop_count1; k++) {
                        test_it();
                        for (j=0; j<loop_count0; j++);
                }
        }
        i = chprio((int) arg, 16);
        assert(i == 64);
        return 0;
}

int test4(void *args)
{
        int pid1, pid2;
        int r;
        int arg = 0;

        (void)args;

        printf("Test 4 ---------------------------------------------------\n");

        assert(getprio(getpid()) == 128);
        pid1 = start(busy1, 4000, 64, "busy1", (void *) arg);
        assert(pid1 > 0);
        pid2 = start(busy2, 4000, 64, "busy2", (void *) pid1);
        assert(pid2 > 0);
        printf("1 -");
        r = chprio(getpid(), 32);
        assert(r == 128);
        printf(" - 2");
        r = kill(pid1);
        assert(r == 0);
        assert(waitpid(pid1, 0) == pid1);
        r = kill(pid2);
        assert(r < 0); /* kill d'un processus zombie */
        assert(waitpid(pid2, 0) == pid2);
        printf(" 3");
        r = chprio(getpid(), 128);
        assert(r == 32);
        printf(" 4.\n");

        printf("\n");
        return 0;
}