#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__

#include <stdio.h>
#include <stdlib.h>
#include "processStructures.h"
#include "queue.h"
#include "mem.h"

/* Total number of queues that can be created */
#define NBQUEUE 256
/* Priority to the processes waiting for a message that are unblocked */
#define UNLOCK_PRIORITY 10

typedef struct __message_queue {
    int buffer_size;
    int* buffer;
    int read_index;
    int write_index;
    int nb_messages;
    int reset_flag;
    link blocked_processes_links;
} message_queue;

/* Number of queues already created in the system */
typedef struct __queue_manager {
    int count;
    message_queue* queues[NBQUEUE];
} queue_manager;

extern link ready_list;
extern int32_t active_PID;

int count_blocked_process(int);
int calc_number_of_messages(int);
int unblock_processes(int);
int check_fid_validity(int);

int pcreate(int);
int pdelete(int);
int pcount(int, int*);
int preceive(int, int*);
int preset(int);
int psend(int, int);

#endif