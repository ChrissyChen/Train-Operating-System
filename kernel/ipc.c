
#include <kernel.h>


PORT_DEF port_defs[MAX_PORTS];

PORT next_free_port;



PORT create_port()
{
	return create_new_port(active_proc);
}


PORT create_new_port (PROCESS owner)
{
	assert(owner->magic == MAGIC_PCB);
	PORT new_port;
	if (next_free_port == NULL) {
		panic("create_new_port(): PORT is full");
	}
	new_port = next_free_port;
	next_free_port = new_port->next;//???

	new_port->magic = MAGIC_PORT;
	new_port->used = TRUE;
	new_port->open = TRUE;
	new_port->owner = owner;
	new_port->blocked_list_head = NULL;
	new_port->blocked_list_tail = NULL;

	if(owner->first_port == NULL) {
		new_port->next = NULL;
	} else {
		new_port->next = owner->first_port; //??
	}
	owner->first_port = new_port;//??

	return new_port;
}




void open_port (PORT port)
{
	assert(port->magic == MAGIC_PORT);
	port->open = TRUE;
}



void close_port (PORT port)
{
	assert(port->magic == MAGIC_PORT);
	port->open = FALSE;
}


void send (PORT dest_port, void* data)
{
	assert(dest_port->magic == MAGIC_PORT);

	PROCESS dest_proc = dest_port->owner;
	assert(dest_proc->magic == MAGIC_PCB);

	if (dest_proc->state == STATE_RECEIVE_BLOCKED && dest_port->open) {
		// Puder:didn't write cuz it is included in add_ready_queue()
		//dest_proc->state = STATE_READY;
		dest_proc->param_proc = active_proc; //???
		dest_proc->param_data = data; //???
		add_ready_queue(dest_proc);

		active_proc->state = STATE_REPLY_BLOCKED;//remove_ready_queue & resign later
	} else {
		add_process_to_send_blocked_list(dest_port, active_proc);
		active_proc->state = STATE_SEND_BLOCKED;
		active_proc->param_data = data;//???
	}

	remove_ready_queue (active_proc);
    resign();
}

void add_process_to_send_blocked_list (PORT port, PROCESS proc)
{
    assert (port->magic == MAGIC_PORT);
    assert (proc->magic == MAGIC_PCB);
    if (port->blocked_list_head == NULL) {
		port->blocked_list_head = proc;
    } else {
		port->blocked_list_tail->next_blocked = proc;
    	port->blocked_list_tail = proc;
    	proc->next_blocked = NULL;
	}
}

void message (PORT dest_port, void* data)
{
	assert(dest_port->magic == MAGIC_PORT);

	PROCESS dest_proc = dest_port->owner;
	assert(dest_proc->magic == MAGIC_PCB);

	if (dest_proc->state == STATE_RECEIVE_BLOCKED && dest_port->open) {
		dest_proc->param_proc = active_proc; //???
		dest_proc->param_data = data; //???
		add_ready_queue(dest_proc);
	} else {
		add_process_to_send_blocked_list(dest_port, active_proc);
		active_proc->state = STATE_MESSAGE_BLOCKED;
		active_proc->param_data = data;//???

		remove_ready_queue (active_proc);
	}
	resign();//???
}



void* receive (PROCESS* sender)
{
	PROCESS      sender_proc;
    PORT         port;
    void         *data;

    data = NULL;
    port = active_proc->first_port;
    if (port == NULL) {
		panic ("receive(): no port created for this process");
	}
	// scan receiver's ports to find the first open port that has procs in its send_blocked_list
    while (port != NULL) {
		assert (port->magic == MAGIC_PORT);
		if (port->open && port->blocked_list_head != NULL) {
	    	break;
		}
		port = port->next;
    }
    
    if (port != NULL) {
		sender_proc = port->blocked_list_head;
		assert (sender_proc->magic == MAGIC_PCB);
		*sender = sender_proc;
		data = sender_proc->param_data;
		port->blocked_list_head = port->blocked_list_head->next_blocked;
		if (port->blocked_list_head == NULL) {
	    	port->blocked_list_tail = NULL;
		}
		if (sender_proc->state == STATE_MESSAGE_BLOCKED) {
	    	add_ready_queue (sender_proc);
	    	return data;
		} else if (sender_proc->state == STATE_SEND_BLOCKED) {
	    	sender_proc->state = STATE_REPLY_BLOCKED;
	    	return data;
		}
    } else {  /* No messages pending */
    	remove_ready_queue (active_proc);
    	active_proc->param_data = data;
    	active_proc->state = STATE_RECEIVE_BLOCKED;
    	resign();
    	*sender = active_proc->param_proc; //???
    	data = active_proc->param_data; //???
    	return data;
	}
}


void reply (PROCESS sender)
{
	if (sender->state != STATE_REPLY_BLOCKED) {
		panic ("reply(): Not reply blocked");
	}
    add_ready_queue (sender);
    resign();
}


void init_ipc()
{
	int i;

    next_free_port = port_defs;
    for (i = 0; i < MAX_PORTS - 1; i++) {
		port_defs[i].used = FALSE;
		port_defs[i].magic = MAGIC_PORT;
		port_defs[i].next = &port_defs[i + 1];
    }
    port_defs[MAX_PORTS - 1].used  = FALSE;
    port_defs[MAX_PORTS - 1].magic = MAGIC_PORT;
    port_defs[MAX_PORTS - 1].next  = NULL;
}
