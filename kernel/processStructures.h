#ifndef __PROCESSSTRUCTURES_H__
#define __PROCESSSTRUCTURES_H__

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "../shared/queue.h"
#include "stdbool.h"

#define MAXNPROC 100
#define SIZE_NAME 20
#define SIZE_CNTXT 5

typedef enum _status {
    ELECTED,
    READY,
    BLOCKED_MESSAGE_QUEUE,
    ASLEEP,
    DYING,
    ZOMBIE,
    BLOCKED
} Status;

typedef struct _proc {
    int32_t pid;
    char name[SIZE_NAME];
    enum _status status;
    int32_t *stack;
    unsigned long stack_size;
    int32_t context[SIZE_CNTXT];
    uint32_t priority;
    link links;
    uint32_t wake_up_time;
    struct _proc* parent;
    link children_list;
    link siblings;
    int retval;
    bool killed;
} Proc;

extern Proc *processes_table[MAXNPROC];

#endif