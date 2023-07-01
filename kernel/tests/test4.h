/*******************************************************************************
 * Test 4
 *
 * Boucles d'attente active (partage de temps)
 * chprio()
 * kill() de processus de faible prio
 * kill() de processus deja mort
 ******************************************************************************/
#ifndef _TEST4_H_
#define _TEST4_H_

#ifdef microblaze
static const int loop_count0 = 500;
static const int loop_count1 = 1000;
#else
static const int loop_count0 = 5000;
static const int loop_count1 = 10000;
#endif

int test4(void *args);
int busy2(void *arg);
int busy1(void *arg);
void test_it(void);

#endif /* _TEST4_H_ */
