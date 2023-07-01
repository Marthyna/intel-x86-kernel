#ifndef _TEST13_H_
#define _TEST13_H_

#include "processManager.h"
#include "debug.h"
#include "processExamples.h"
#include "message_queue.h"
#include "shared_memory.h"

struct psender {
    int fid;
    char data[32];
};

int test13(void *arg);
int preceiver_msg(void *arg);
int psender_msg(void *arg);

#endif /* _TEST13_H_ */