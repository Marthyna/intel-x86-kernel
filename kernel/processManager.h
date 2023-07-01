#ifndef __PROCESSMANAGER_H__
#define __PROCESSMANAGER_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mem.h"
#include "processStructures.h"
#include "queue.h"
#include "string.h"
#include "timer.h"

#define DEFAULT_PRIORITY 2
#define DEFAULT_SSIZE 400
#define MIN_SSIZE 3

extern link ready_list;
extern link asleep_list;
extern link dying_list;
extern link zombie_list;
extern link blocked_list;

extern void ctx_sw(int32_t *old_proc, int32_t *new_proc);  // prototype function assembly(ctx_sw.S)
extern void exit_handler(void);
int start(int (*pt_func)(void *), unsigned long ssize, int prio, const char *name, void *arg);

int32_t get_new_pid();
Proc *get_parent_proc();
int waitpid(int pid, int *retvalp);
Proc *wait_for_child();

int32_t elect_proc(int32_t current);
void scheduling(void);

void wakeup();
void wait_clock(unsigned long);
void sleep(uint32_t nbr_secs);

int chprio(int, int);
int getprio(int);
int32_t getpid(void);
char* my_name(void);

void kill_processes(void);
void exit(int retval);
void free_children(Proc *current_process);
int kill(int pid);
bool in_children_list(int pid);
bool parent_still_alive(Proc* process);
Proc* get_terminated_child();
int throw_start_error(char* message, int pid);

#endif