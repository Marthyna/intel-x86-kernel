#include "processLists.h"
#include "processManager.h"

Proc* processes_table[MAXNPROC];

LIST_HEAD(ready_list);
LIST_HEAD(asleep_list);
LIST_HEAD(dying_list);
LIST_HEAD(zombie_list);
LIST_HEAD(blocked_list);

void print_process_queue(link *queue) {
    Proc* it = queue_top(queue, Proc, links);
    if(it != NULL) {
        printf("Beginning of the queue: %d\n", it->status);
        queue_for_each(it, queue, Proc, links) {    
            printf("\tPID: %d, Name: %s\n", it->pid, it->name);
        }
        printf("End of queue\n");
    } else {
        printf("Empty queue\n");
    }
}

void change_proc_list(Proc* proc, link dst_queue) {
    // delete from src_queue
    queue_del(proc, links);

    // put in the destination queue
    queue_add(proc, &dst_queue, Proc, links, priority);
}

bool in_list(link *queue, int pid) {
    Proc *process;
    queue_for_each(process, queue, Proc, links) {
        if(process->pid == pid)
            return true;
    }
    return false;
}