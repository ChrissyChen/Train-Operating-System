// Student Name:  Xinlu Chen  
// Student ID:    915757497

#include <kernel.h>

#define TRAIN_ID "20"
#define TRAIN_CR '\015'
#define TICK_NUM 8
#define MAX_ARRAY_LEN 10
#define INPUT_BUFFER_LEN 3


void send_command (char* command, char* input_buffer, int input_len)
{
	COM_Message msg;
	msg.output_buffer = command;
	msg.input_buffer = input_buffer;
	msg.len_input_buffer = input_len;
	send (com_port, &msg);	
}


// Clear the s88 memory buffer
void clear_s88_buffer (int window_id)
{
	char input_buffer;
	char command[MAX_ARRAY_LEN];
	char *ptr = command;

	command[0] = 'R';
	command[1] = '\0';
	//int string_len = k_strlen (command);
	//wm_print (window_id, "Command length: %d\n", string_len);
	wm_print (window_id, "s88 memory buffer cleaned (%s)\n", command);

	command[1] = TRAIN_CR;
	command[2] = '\0';
	//string_len = k_strlen (command);
	//wm_print (window_id, "Command length: %d\n", string_len);
	//wm_print (window_id, "s88 memory buffer cleaned (%s)\n", command);

	send_command (ptr, &input_buffer, 0);
}


// Probe a contact to know if any vehicle is on it
void probe_contact (char* contact_id, int window_id)
{
	clear_s88_buffer (window_id);

	char input_buffer[INPUT_BUFFER_LEN];	
	char command[MAX_ARRAY_LEN];
	char *ptr = command;
	int len = k_strlen (contact_id);

	command[0] = 'C';
	k_memcpy (&command[1], contact_id, len);
	command[++len] = TRAIN_CR;
	command[++len] = '\0';
	send_command (ptr, &input_buffer[0], INPUT_BUFFER_LEN);

	// Get the COM1 returned result via com_reader process
	char result = input_buffer[1];
	command[0] = 'C';
	k_memcpy (&command[1], contact_id, len);
	command[++len] = '\0';
	wm_print (window_id, "Probe result of contact %s: %c (%s)\n", contact_id, result, command);
}


// Change the direction of a train
void set_direction (int window_id)
{
	char input_buffer;
	char command[MAX_ARRAY_LEN];
	char *ptr = command;
	int len = k_strlen (TRAIN_ID);
	//wm_print (window_id, "TRAIN_ID len: %d\n", len);

	command[0] = 'L';
	k_memcpy (&command[1], TRAIN_ID, len);
	command[++len] = 'D';
	command[++len] = '\0';

	//int string_len = k_strlen (command);
	//wm_print (window_id, "Command length: %d\n", string_len);
	wm_print (window_id, "Reversed direction of train (%s)\n", command);
	
	command[len] = TRAIN_CR;
	command[++len] = '\0';
	//string_len = k_strlen (command);
	//wm_print (window_id, "Command length: %d\n", string_len);
	//wm_print (window_id, "Reversed direction of train (%s)\n", command);
	
	send_command (ptr, &input_buffer, 0);
}


// Change the speed of a train
void set_speed (char speed, int window_id)
{
	char input_buffer;
	char command[MAX_ARRAY_LEN];
	char *ptr = command;
	int len = k_strlen (TRAIN_ID);
	//wm_print (window_id, "TRAIN_ID len: %d\n", len);

	command[0] = 'L';
	k_memcpy (&command[1], TRAIN_ID, len);
	command[++len] = 'S';
	command[++len] = speed;
	command[++len] = '\0';

	//int string_len = k_strlen (command);
	//wm_print (window_id, "Command length: %d\n", string_len);
	wm_print (window_id, "Changed train velocity to %c (%s)\n", speed, command);
	
	command[len] = TRAIN_CR;
	command[++len] = '\0';
	//string_len = k_strlen (command);
	//wm_print (window_id, "Command length: %d\n", string_len);
	//wm_print (window_id, "Changed train velocity to %c (%s)\n", speed, command);
	
	send_command (ptr, &input_buffer, 0);
}


// 	Set the switch to a certain direction
void set_switch (char switch_id, char color, int window_id)
{
	char input_buffer;
	char command[MAX_ARRAY_LEN];
	char *ptr = command;

	command[0] = 'M';
	command[1] = switch_id;
	command[2] = color;
	command[3] = '\0';
		
	//int string_len = k_strlen (command);
	//wm_print (window_id, "Command length: %d\n", string_len);
	wm_print (window_id, "Changed switch %c to %c (%s)\n", switch_id, color, command);

	command[3] = TRAIN_CR;
	command[4] = '\0';
	//string_len = k_strlen (command);
	//wm_print (window_id, "Command length: %d\n", string_len);
	
	//will has a garbage character because shell can't recognize '\015' but train can recognize it as a terminator
	//wm_print (window_id, "Changed switch %c to %c (%s)\n", switch_id, color, command);
	
	send_command (ptr, &input_buffer, 0);
}


// Initialize the switches so that Zamboni, if exists, will not run out of track
void init_switch (int window_id)
{
	wm_print (window_id, "Initializing the switches... Done!\n");
	set_switch ('4', 'G', window_id);
	set_switch ('5', 'G', window_id);
	set_switch ('8', 'G', window_id);
	set_switch ('9', 'R', window_id);
	set_switch ('1', 'G', window_id);
}


// For functionality testing only - test under config 1
void test_train_command (int window_id)
{
	set_speed ('5', window_id);
	sleep (100);
	set_speed ('0', window_id);
	sleep (100);
	set_direction (window_id);
	set_speed ('5', window_id);

	probe_contact ("7", window_id);
	sleep (20);
	probe_contact ("10", window_id);
	sleep (100);
	probe_contact ("14", window_id);
	sleep (100);
	probe_contact ("3", window_id);
	
}


// Entry point of the train process
void train_process(PROCESS self, PARAM param)
{
	int window_id = wm_create (12, 5, 60, 17);
	wm_print (window_id, "****** Welcome to Train Application ******\n\n");
	init_switch (window_id);
	test_train_command (window_id);
}


void init_train()
{
	create_process (train_process, 5, 0, "Train Process");
	resign ();
}
