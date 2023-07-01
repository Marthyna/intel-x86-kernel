#ifndef __PROCESSLISTS_H__
#define __PROCESSLISTS_H__

#include <stdbool.h>

#include "processStructures.h"

void print_process_queue(link*);
void change_proc_list(Proc*, link);
bool in_list(link *queue, int pid);

#endif