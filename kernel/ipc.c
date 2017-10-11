
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
		new_port->next = owner->first_owner; //??
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
}


void message (PORT dest_port, void* data)
{
}



void* receive (PROCESS* sender)
{
}


void reply (PROCESS sender)
{
}


void init_ipc()
{
}
