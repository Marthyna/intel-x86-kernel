#include "message_queue.h"
#include "processManager.h"

queue_manager queue_data = {
    /* no lists are created so far */
    .count = 0,
    /* so, initialize the array to contain all NULLs */
    .queues = {NULL}};

/* Reads data and processes waiting on the fid queue. */
int pcount(int fid, int *count) {
    message_queue *queue;

    /* if fid is invalid or greater than the allowed number of queues, error  */
    if (!check_fid_validity(fid) || fid >= NBQUEUE)
        return -1;

    /* if count is not null, calculate the count value */
    if (count != NULL) {
        queue = queue_data.queues[fid];
        /* if empty queue, returns negative value equal to the opposite of the number of processes blocked */
        if (queue->nb_messages == 0)
            *count = count_blocked_process(fid) * -1;
        else
            /* otherwise, return the number of messages + blocked processes */
            *count = queue->nb_messages + count_blocked_process(fid);
    }
    return 0;
}

int count_blocked_process(int fid) {
    check_fid_validity(fid);
    
    // message_queue* queue = queue_data.queues[fid];
    Proc* cur_element;
    int i=0;
    /* count the number of blocked items */
    queue_for_each(cur_element, &(queue_data.queues[fid]->blocked_processes_links), Proc, links) {
        i++;
    }
    return i;
}

/* allocates a queue of capacity equal to the value of count */
int pcreate(int count) {
    if (count <= 0 || queue_data.count >= NBQUEUE || count>((int)(2147483647/sizeof(int))))
        return -1;

    /* alloc list space */
    message_queue *mem_allocated = (message_queue *)mem_alloc(sizeof(message_queue));
    if(mem_allocated == 0)
        return -1;

    int available_fid;
    /* find the first available fid */
    for (available_fid = 0; available_fid < NBQUEUE; available_fid++)
        if (queue_data.queues[available_fid] == NULL)
            break;

    /* store in the structure */
    queue_data.queues[available_fid] = mem_allocated;

    /* initialize values */
    mem_allocated->buffer = (int *)mem_alloc(sizeof(int) * count);
    if(mem_allocated->buffer == 0)
        return -1;
    mem_allocated->buffer_size = count;
    mem_allocated->read_index = 0;
    mem_allocated->write_index = 0;
    mem_allocated->nb_messages = 0;
    mem_allocated->reset_flag = 0;

    /* lists initializiation */
    INIT_LIST_HEAD(&(mem_allocated->blocked_processes_links));

    /* update the queues count */
    queue_data.count++;

    /* return the index that identifies the queue in the structure */
    return available_fid;
}

int unblock_processes(int fid) {

    while(!list_is_empty(&(queue_data.queues[fid]->blocked_processes_links))) {
        Proc* cur = queue_top(&(queue_data.queues[fid]->blocked_processes_links), Proc, links);
        queue_del(cur, links);
        queue_add(
            cur,  // remove the element from the list
            &ready_list,
            Proc,
            links,
            priority
        );
        cur->status = READY;
    }

    // queue_for_each(current_process, &blocked_processes_link, Proc, links) {
    //     // change from the blocked to ready
    //     current_process->status = READY;
    //     queue_add(
    //         queue_out(&blocked_processes_link, Proc, links),  // remove the element from the list
    //         &ready_list,
    //         Proc,
    //         links,
    //         priority);       
    // }

    return 1;
}

/* destroys the fid and set to ready all the processes that were stuck on the queue*/
int pdelete(int fid) {
    if (!check_fid_validity(fid))
        return -1;

    unblock_processes(fid);

    // free the memory space
    mem_free(queue_data.queues[fid]->buffer, sizeof(int) * queue_data.queues[fid]->buffer_size);
    // put the pointer to null once again
    queue_data.queues[fid] = NULL;
    queue_data.count--;

    scheduling();

    return 0;
}

int check_fid_validity(int fid) {
    if (fid < 0 || queue_data.queues[fid] == NULL) {
        return 0;
    }
    return 1;
}

/* reads and removes the first (oldest) message from the fid queue */
int preceive(int fid, int *message) {
    message_queue *queue;

    if (!check_fid_validity(fid)) {
        return -1;
    }

    queue = queue_data.queues[fid];

    // if there is no message to be read, block and wait
    if (queue->nb_messages == 0) {
        
        // block the process
        Proc *current = processes_table[active_PID];
        current->status = BLOCKED_MESSAGE_QUEUE;
        queue_add(current, &(queue->blocked_processes_links), Proc, links, priority);
        
        // and call the scheduler
        scheduling();

        // if he comes back and still there is no message to be read, the return is negative
        // if (queue == NULL || queue->nb_messages == 0 || queue->reset_flag--)
        if(queue_data.queues[fid] == NULL || queue->nb_messages == 0 || (queue->reset_flag!=0)) {
            queue->reset_flag--;
            return -1;
        }
    }

    // read the message
    *message = queue->buffer[queue->read_index];
    queue->read_index = (queue->read_index + 1) % queue->buffer_size;
    queue->nb_messages--;

    // if there are processes waiting to put a message on the queue
    if (count_blocked_process(fid) > 0 && queue->nb_messages == queue->buffer_size-1) {
        // we'll go ahead and put them in the ready list
        Proc *head_blocked = queue_out(&(queue->blocked_processes_links), Proc, links);

        // the process can be killed and no longer make part of the blocked list
        if(head_blocked != NULL) {
            head_blocked->status = READY;
            queue_add(head_blocked, &ready_list, Proc, links, priority);

            // if it is the one with the most priority, it should execute
            scheduling();
        }
    }

    return 0;
}

/* empties the fid queue and unblocks all processes */
int preset(int fid) {
    message_queue *message_queue;

    if (!check_fid_validity(fid)) {
        return -1;
    }

    // get the queue
    message_queue = queue_data.queues[fid];


    // to empty the list, we just have to set the both indexes to the start
    message_queue->read_index = 0;
    message_queue->write_index = 0;
    message_queue->nb_messages = 0;
    message_queue->reset_flag = count_blocked_process(fid);

    unblock_processes(fid);

    scheduling();

    return 0;
}

/* sends the message to the queue identified by fid */
int psend(int fid, int message) {
    message_queue *queue;
    int schedule_needed = 0;

    if (!check_fid_validity(fid)) {
        return -1;
    }

    queue = queue_data.queues[fid];

    // check if list is empty and if there are process waiting
    if (queue->nb_messages == 0 && count_blocked_process(fid) > 0) {

        Proc *blocked_proc = queue_out(&(queue->blocked_processes_links), Proc, links);
        
        // the process can be killed and no longer make part of the blocked list
        if(blocked_proc != NULL) {
            //chprio(blocked_proc->pid, UNLOCK_PRIORITY);

            // put in the ready list
            blocked_proc->status = READY;
            queue_add(blocked_proc, &ready_list, Proc, links, priority);

            schedule_needed = 1;
        }
    }
    // if the list is full then block the process until it can put the message
    else if (queue->nb_messages == queue->buffer_size) {
        while (queue->nb_messages == queue->buffer_size) {
            Proc *current = processes_table[active_PID];
            current->status = BLOCKED_MESSAGE_QUEUE;
            queue_add(current, &(queue->blocked_processes_links), Proc, links, priority);

            scheduling();

            // check if preset or pdelete was called
            if(queue_data.queues[fid] == NULL || (queue->reset_flag!=0 && queue->reset_flag--)) {
                return -1;
            }
        }
    }
    // write the message
    queue->buffer[queue->write_index] = message;
    queue->write_index = (queue->write_index + 1) % queue->buffer_size;
    queue->nb_messages++;

    if(schedule_needed)
        scheduling();

    return 0;
}