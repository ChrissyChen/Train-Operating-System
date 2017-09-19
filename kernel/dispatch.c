
#include <kernel.h>

#include "disptable.c"


PROCESS active_proc;


/*
 * Ready queues for all eight priorities.
 */
PCB *ready_queue [MAX_READY_QUEUES];


/*
 * Puder: The bits in ready_procs tell which ready queue is empty.
 * The MSB of ready_procs corresponds to ready_queue[7].
 */
unsigned ready_procs;


/*
 * add_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by p is put the ready queue.
 * The appropiate ready queue is determined by p->priority.
 */

void add_ready_queue (PROCESS proc)
{
	//puder: check if PROCESS is valid
	assert(proc->magic == MAGIC_PCB);
	int prio = proc->priority;
	proc->state = STATE_READY;
	if (ready_queue[prio] == NULL ) {
		ready_queue[prio] = proc;
		proc->next = proc;
		proc->prev = proc;
		ready_procs |= 1 << prio;
	} else {
		proc->next = ready_queue[prio];
		proc->prev = ready_queue[prio]->prev; //order matters!
		ready_queue[prio]->prev->next = proc;
		ready_queue[prio]->prev = proc;	
	}
}



/*
 * remove_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by p is dequeued from the ready
 * queue.
 */

void remove_ready_queue (PROCESS proc)
{
	assert(proc->magic == MAGIC_PCB);
	int prio = proc->priority;
	if (proc->next == proc) {
		ready_queue[prio] = NULL;
		ready_procs ^= 1 << prio; //puder: ... &= ~(1<<prio);
	} else {
		ready_queue[prio] = proc->next;
		proc->prev->next = proc->next;
		proc->next->prev = proc->prev;
	}
}



/*
 * dispatcher
 *----------------------------------------------------------------------------
 * Determines a new process to be dispatched. The process
 * with the highest priority is taken. Within one priority
 * level round robin is used.
 */
// Puder's answer
PROCESS dispatcher()
{
	PROCESS new_proc;
	unsigned i;

	i = table[ready_procs];
	assert(i != -1);
	if (active_proc->priority == i) {
		new_proc = active_proc->next;
	} else {
		new_proc = ready_queue[i];
	}
	return new_proc;
}



/*
 * resign
 *----------------------------------------------------------------------------
 * The current process gives up the CPU voluntarily. The
 * next running process is determined via dispatcher().
 * The stack of the calling process is setup such that it
 * looks like an interrupt.
 */
void resign()
{
}



/*
 * init_dispatcher
 *----------------------------------------------------------------------------
 * Initializes the necessary data structures.
 */

void init_dispatcher()
{
	for (int i = 0; i < MAX_READY_QUEUES; i++) {
			ready_queue[i] = NULL;
	}
	ready_procs = 0;
	//set up first process
	add_ready_queue(active_proc);
}
