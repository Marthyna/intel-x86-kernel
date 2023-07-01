#include "processManager.h"
#include "processExamples.h"
#include "processLists.h"
#include "processStructures.h"
#include "processManager.h"
#include "message_queue.h"

unsigned int nb_proc = 0;
int32_t active_PID = 0;
extern queue_manager queue_data;    

int32_t get_new_pid() {
    int32_t pid;
    for (pid = 0; pid < MAXNPROC && processes_table[pid] != NULL; pid++);
    if (pid == MAXNPROC){
        pid = -1; // On a dépassé le nombre maximal de processus
    }
    return pid;
}

Proc *get_parent_proc() {
    if (nb_proc == 0) {
        return NULL;
    } else {
        return processes_table[active_PID];
    }
}

void filiationManager(Proc* newProc){
    INIT_LIST_HEAD(&(newProc->children_list));
    newProc->parent = get_parent_proc();
    if (nb_proc > 0){ // if not first process
        queue_add(
            newProc,
            &(newProc->parent->children_list),
            Proc,
            siblings,
            priority);
    }
}

int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg) {
    int32_t pid = get_new_pid();

    // get_new_pid returned -1 because we exceeded the maximum number of processes
    if (pid < 0) {
        return -1;
    }

    processes_table[pid] = (Proc *)mem_alloc(sizeof(Proc));
    if (processes_table[pid] == NULL) {
        printf("Fatal: failed to allocate Proc*\n");
        return -1;
    }

    unsigned long real_stack_size;    
    // Treat arithmetic overflow case
    if (__builtin_add_overflow(ssize, MIN_SSIZE, &real_stack_size)) {
        return throw_start_error("Fatal: overflow on calculating stack size\n", pid);
    }

    // check if minimum stack size attended
    if (real_stack_size < DEFAULT_SSIZE)
        real_stack_size = DEFAULT_SSIZE;
    processes_table[pid]->stack_size = real_stack_size;

    // allocate memory to the stack, with size given by ssize + 3
    processes_table[pid]->stack = (int32_t *)mem_alloc((processes_table[pid]->stack_size) * sizeof(int32_t));
    if (processes_table[pid]->stack == NULL) {
        return throw_start_error("Fatal: failed to allocate process stack\n", pid);
    }

    // Elect idle directly
    processes_table[pid]->status = pid == 0 ? ELECTED : READY;
    
    // set structure
    processes_table[pid]->pid = pid;
    strcpy(processes_table[pid]->name, name);
    processes_table[pid]->priority = prio;
    processes_table[pid]->wake_up_time = 0;

    // Initialize the stack
    processes_table[pid]->stack[real_stack_size - 1] = (int32_t)arg;
    processes_table[pid]->stack[real_stack_size - 2] = (int32_t)exit_handler;
    processes_table[pid]->stack[real_stack_size - 3] = (int32_t)pt_func;
    processes_table[pid]->context[1] = (int32_t) & (processes_table[pid]->stack[real_stack_size - 3]);

    filiationManager(processes_table[pid]);
    
    // add the process to the ready list by default
    queue_add(processes_table[pid], &ready_list, Proc, links, priority);
    nb_proc++;

    if(processes_table[pid]->priority > processes_table[active_PID]->priority)
        scheduling();

    return processes_table[pid]->pid;
}

int throw_start_error(char* message, int pid) {
    printf("%s", message);
    // remove from process table
    Proc* process = processes_table[pid];
    processes_table[pid] = NULL;
    // free the memory
    mem_free((void *)process, sizeof(Proc));
    return -1;
}

int32_t elect_proc(int32_t current) {
    int32_t next_pid;

    if (processes_table[current]->status == ELECTED) {
        processes_table[current]->status = READY;
        if(!in_list(&ready_list, current))
            queue_add(processes_table[current], &ready_list, Proc, links, priority);
    }

    // the new elected process is going to be the next in the ready list
    Proc *next_proc = queue_out(&ready_list, Proc, links);

    next_pid = next_proc->pid;

    processes_table[next_pid]->status = ELECTED;

    if(active_PID == next_pid) 
        return -1;

    // update active pid
    active_PID = next_pid;

    return next_pid;
}

void psHierarchy(Proc* proc, int level){
    if (level == 0){
        printf("-+= %04d %s\n", proc->pid, proc->name);
    }
    else{
        for(int i = 0; i < level; i++)  printf(" ");
        printf("|--= %04d %s\n", proc->pid, proc->name);
    }
    Proc* it;
    queue_for_each(it, &(proc->children_list), Proc, siblings) {
        psHierarchy(it, level+1);
    }
}
void pstree(){
    psHierarchy(processes_table[0], 0);
}

void scheduling(void) {
    int32_t current = active_PID;
    int32_t next;

    wakeup();
    kill_processes();
    next = elect_proc(current);

    if(next == -1)
        return;

    ctx_sw(
        processes_table[current]->context,
        processes_table[next]->context);
}

void wakeup() {
    Proc *process = NULL;
    Proc* processes_to_wake_up[MAXNPROC];
    int i = 0;

    // Go through list of asleep processes
    queue_for_each(process, &asleep_list, Proc, links) {
        // If it's time to wake up, add process to table of procs to wake up
        if(current_clock() > process->wake_up_time) {
            processes_to_wake_up[i] = process;
            i++;
        }
    }

    // Changes processes status outsite the queue loop to avoid link breaking
    for (int j = 0; j < i; j++) {
        processes_to_wake_up[j]->status = READY;
        processes_to_wake_up[j]->wake_up_time = 0;
        queue_del(processes_to_wake_up[j], links);
        queue_add(processes_to_wake_up[j], &ready_list, Proc, links, priority);
    }
}

void sleep(uint32_t nbr_secs) {
    wait_clock(current_clock() + nbr_secs*50);
}


void wait_clock(unsigned long clock){
    // set wake up time
    processes_table[active_PID]->wake_up_time = clock;
    processes_table[active_PID]->status = ASLEEP;
    // move to asleep list
    queue_add(processes_table[active_PID], &asleep_list, Proc, links, wake_up_time);

    // call the scheduler to change active process
    scheduling();
}

int getprio(int pid) {
    // check if pid is valid
    if (pid < 0 || pid > MAXNPROC || processes_table[pid] == NULL)
        return -1;
    return processes_table[pid]->priority;
}

int chprio(int pid, int newprio) {
    int oldprio = getprio(pid);
    
    // If the value of newprio or pid is invalid, the return value of chprio is strictly negative
    //if try change zombie priority
    if (oldprio <= 0 || (pid > 0 && newprio <= 0) || processes_table[pid]->status == ZOMBIE)
        return -1;
    
    //chprio gives newprio priority to the process identified by the pid value.
    Proc *proc = processes_table[pid];
    proc->priority = newprio;

    //update the order of the queue 
    //zombie_list and  dying_list -> dont change 
    //asleep_list will change when the process wakes up 
    switch (proc->status){
    case READY:
        queue_del(proc, links);
        queue_add(proc, &ready_list, Proc, links, priority);
        break;
    
    case BLOCKED:
        queue_del(proc, links);
        queue_add(proc, &blocked_list, Proc, links, priority);
        break;
    
    case BLOCKED_MESSAGE_QUEUE:
        for(int i=0; i<NBQUEUE; i++) {
            // if the queue was not initialized, just continue
            if(queue_data.queues[i] == NULL)
                continue;
            // find the list it is in, and put it with the new priority
            if(in_list(&(queue_data.queues[i]->blocked_processes_links), proc->pid)) {
                queue_del(proc, links);
                queue_add(proc, &(queue_data.queues[i]->blocked_processes_links), Proc, links, priority);
                break;
            }
        }
        // if doesn't find, do nothing
        break;

    default:
        break;
    }

    scheduling();

    //otherwise it is equal to the old priority of the pid process.
    return oldprio;
}

int32_t getpid(void) {
    return processes_table[active_PID]->pid;
}

char *my_name(void) {
    return processes_table[active_PID]->name;
}

void exit(int retval) {
    // Set the return value for the current process
    processes_table[active_PID]->retval = retval;
    processes_table[active_PID]->killed = false;

    // treat children accordingly
    free_children(processes_table[active_PID]);
    
    // if the parent is still alive, set process to zombie state
    if (parent_still_alive(processes_table[active_PID])) {
        processes_table[active_PID]->status = ZOMBIE;
        queue_add(processes_table[active_PID], &zombie_list, Proc, links, priority);

        // if parent is blocked by waitpid, unblock it
        if(processes_table[active_PID]->parent->status == BLOCKED) {
            Proc* parent = processes_table[active_PID]->parent;
            parent->status = READY;

            // deleting parent from its current list changes the children list of the child parent (??)
            queue_del(parent, links);
            
            queue_add(parent, &ready_list, Proc, links, priority);
        }
    } else {
        // if not, move the process directly to the dying list
        processes_table[active_PID]->status = DYING;
        queue_add(processes_table[active_PID], &dying_list, Proc, links, priority);
    }

    // call the scheduler to change active process
    scheduling();

    while(1);
}

/* Terminates all processes in the dying list, freeing their memory */
void kill_processes(void) {
    Proc* proc2kill;
    // while there is at least another process to kill besides the current one, go through the list
    while(!list_is_empty(&dying_list) && queue_bottom(&dying_list, Proc, links) != processes_table[active_PID])
        queue_for_each(proc2kill, &dying_list, Proc, links) {            
            // remove process from the dying list
            queue_del(proc2kill, links);

            // remove from process table
            processes_table[proc2kill->pid] = NULL;

            int32_t *stack = proc2kill->stack;
            unsigned long stack_size = proc2kill->stack_size;

            // free the memory
            mem_free((void *)proc2kill, sizeof(Proc));
            // free the reserved stack size
            mem_free((void *)stack, sizeof(int) * stack_size);

            // update total number of processes counter
            nb_proc--;

            break;            
        }
}

/* each child of the terminated process is marked as having no parent
   and the zombie children are destroyed. */
void free_children(Proc *current_process) {
    Proc *current_child;

    queue_for_each(current_child, &(current_process->children_list), Proc, siblings) {
        current_child = queue_out(&(current_process->children_list), Proc, siblings);
        if (current_child == NULL)
            return;

        // zombie children are destroyed
        if (current_child->status == ZOMBIE) {
            current_child->status = DYING;
            queue_del(current_child, links);
            queue_add(current_child, &dying_list, Proc, links, priority);
        } else {
            // and normal children lose their parents (parent is now the idle proc)
            current_child->parent = processes_table[0];
        }
    }
}

/* Waiting for and retrieving the return value of a child process */
int waitpid(int pid, int *retvalp) {
    Proc *child;

    // If the pid is negative, wait for any child to terminate
    if (pid < 0) {
        child = wait_for_child();

        // If no children, return error
        if (child == NULL)
            return -1;
    } else {
        // If the pid is positive, wait for the child with the given pid to terminate
        child = processes_table[pid];

        if (child == NULL || !in_children_list(child->pid)) {
            // No child process with the given pid exists or is not child of current process
            return -1;
        }

        // Wait for the child process to terminate
        while (child->status != ZOMBIE) {
            // If child has not terminated yet, block parent
            processes_table[active_PID]->status = BLOCKED;
            queue_add(processes_table[active_PID], &blocked_list, Proc, links, priority);

            scheduling();
        } 
    }
    // Once child has terminated, retrieve the return value of the child process
    if (retvalp != NULL) {
        if (child->killed) {
            *retvalp = 0;
        } else {
        *retvalp = child->retval;
        }
    }
    // Terminate zombie child
    child->status = DYING;
    queue_del(child, links);
    queue_del(child, siblings);
    queue_add(child, &dying_list, Proc, links, priority);
    return child->pid;
}

/* checks if given pid is a child of current process */
bool in_children_list(int pid) {
    Proc *child;
    queue_for_each(child, &(processes_table[active_PID]->children_list), Proc, siblings) {
        if (child->pid == pid)
            return true;
    }
    return false;
}

// Wait for any child to terminate
Proc *wait_for_child() {
    Proc *child;
    if (list_is_empty(&(processes_table[active_PID]->children_list)))
        return NULL;

    child = get_terminated_child();

    if(!child) {
        // If no child has terminated yet, block parent
        processes_table[active_PID]->status = BLOCKED;
        queue_add(processes_table[active_PID], &blocked_list, Proc, links, priority);

        scheduling();

        // When parent is unblocked by child termination, get terminated child
        child = get_terminated_child();
    }
    return child;
}

// Return first zombie child
Proc* get_terminated_child() {
    Proc *child;
    // Traverse the list of child processes ONCE
    queue_for_each(child, &(processes_table[active_PID]->children_list), Proc, siblings) {
        // If the child process has terminated, return a pointer to it
        if (child->status == ZOMBIE) {
            return child;
        }
    }
    return NULL;
}

/* terminates the process with pid value */
int kill(int pid) {    
    Proc *process = processes_table[pid];
    if (pid <= 0 || process == NULL || process->status == ZOMBIE)
        return -1;

    if(process->status == ZOMBIE)
        return -1;

    process->killed = true;

    if(pid == active_PID)
        exit(0);

    // if the parent is still alive, set process to zombie state
    if (parent_still_alive(process)) {
        process->status = ZOMBIE;
        queue_del(process, links);
        queue_add(process, &zombie_list, Proc, links, priority);
    } else {
        // if not, move the process directly to the dying list
        process->status = DYING;
        queue_del(process, links);
        queue_add(process, &dying_list, Proc, links, priority);
    }
    // treat children accordingly
    free_children(process);

    // call the scheduler to change active process
    scheduling();

    return 0;
}

/* parent still alive if not null and not idle */
bool parent_still_alive(Proc* process) {
    return process->parent != NULL && process->parent->pid != 0;
}