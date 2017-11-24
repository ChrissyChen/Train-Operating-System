// Student Name:  Xinlu Chen  
// Student ID:    915757497

#include <kernel.h>

#define MAX_COMMAND_LEN 30
#define MAX_COMMAND_HISTORY 100

typedef struct _Buffer_Command
{
	char buffer[MAX_COMMAND_LEN];
	int length;
} Buffer_Command;

void execute_command (int window_id, Buffer_Command *command)
{
	
}


BOOL read_command (int window_id, Buffer_Command *command)
{
	BOOL exceed_limit = FALSE;
	command->length = 0;
	char keystroke = keyb_get_keystroke (window_id, TRUE);

	while (keystroke != '\r') {
		switch (keystroke) {
			case '\b':
			case 9:
				if (command->length != 0) {
					command->length--;
					wm_print (window_id, "%c", keystroke); // ???
				}
				break;
				
			default: // including whitespaces
				if (command->length >= MAX_COMMAND_LEN) {
					exceed_limit = TRUE;
					wm_print (window_id, "\n[Error] Max command length is %d\n", MAX_COMMAND_LEN);
					return exceed_limit;
				} else {
					command->buffer[command->length] = keystroke;
					command->length++;
					wm_print (window_id, "%c", keystroke);
				}
				break;
		}

		keystroke = keyb_get_keystroke (window_id, TRUE);
	}

	command->buffer[command->length] = '\0';
	return exceed_limit;
}


void clear_command_buffer (Buffer_Command *command)
{
	while (command->length != 0) {
		command->buffer[command->length - 1] = ' ';
		command->length--;
	}
}


void print_command (int window_id, Buffer_Command *command)
{
	wm_print (window_id, "\n[Test] You just typed: \n");
	for (int i = 0; i < command->length; i++) {
		wm_print (window_id, "%c", command->buffer[i]);
	}
}


void print_welcome (int window_id)
{
	wm_print (window_id, "****** Welcome to TOS Shell ******\n");
	wm_print (window_id, "Type [help] to show all TOS commands\n");
}


void shell_process (PROCESS self, PARAM param)
{
	Buffer_Command *command;
	Buffer_Command *history_command[MAX_COMMAND_HISTORY];
	int i = 0;
	BOOL exceed_limit; // do not need to init to FALSE?

	int window_id = wm_create (10, 3, 50, 17);
	print_welcome (window_id);
	
	while (1) {
		wm_print (window_id, "\n> ");
		exceed_limit = read_command (window_id, command);
		if (exceed_limit == FALSE) {
			history_command[i] = command;
			print_command (window_id, command);

			//remove_whitespace (command);
			//execute_command (window_id, command);	
		} 
		clear_command_buffer (command);
	}
}


// init shell process
void start_shell ()
{
	create_process (shell_process, 5, 0, "Shell Porcess");
	resign ();
}
