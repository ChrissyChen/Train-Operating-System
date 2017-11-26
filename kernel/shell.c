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


void help (int window_id)
{
	wm_print (window_id, "\nTOS commands: \n");
	wm_print (window_id, "help      print all supported TOS commands\n");
	wm_print (window_id, "cls       clear the screen/window\n");
	wm_print (window_id, "shell     launch another shell\n");
	wm_print (window_id, "pong      launch the PONG game\n");
	wm_print (window_id, "ps        print out the process table\n");
	wm_print (window_id, "history   print all commands that have beeen typed\n");
	wm_print (window_id, "!<number> repeat the command with the given number\n");
	wm_print (window_id, "about     print out developer's name");
}



// Remove leading and trailing whitespaces. Only support one-word command now.
void remove_whitespace (Buffer_Command *command, Buffer_Command *removed_command)
{
	removed_command->length = 0;
	int i, start, end;
	for (i = 0; i < command->length; i++) {
		if (command->buffer[i] != ' ') break;
	}
	start = i;

	for (i = command->length - 1; i >= 0; i--) {
		if (command->buffer[i] != ' ') break;
	}
	end = i;

	for (int j = 0, i = start; i <= end; i++) {
		removed_command->buffer[j++] = command->buffer[i];
		removed_command->length++;
	}
}


void execute_command (int window_id, Buffer_Command *removed_command)
{
	//wm_print (window_id, "\nRemoved command: %s", removed_command->buffer);
	if (k_memcmp (removed_command->buffer, "help", k_strlen("help")) == 0) {
		help (window_id);
	} else if (k_memcmp (removed_command->buffer, "cls", k_strlen("cls")) == 0) {
		wm_print (window_id, "\nyes cls");
	} else if (k_memcmp (removed_command->buffer, "shell", k_strlen("shell")) == 0) {
		wm_print (window_id, "\nyes shell");
	} else if (k_memcmp (removed_command->buffer, "pong", k_strlen("pong")) == 0) {
		wm_print (window_id, "\nyes pong");
	} else if (k_memcmp (removed_command->buffer, "ps", k_strlen("ps")) == 0) {
		wm_print (window_id, "\nyes ps");
	} else if (k_memcmp (removed_command->buffer, "history", k_strlen("history")) == 0) {
		wm_print (window_id, "\nyes history");
	} else if (k_memcmp (removed_command->buffer, "!", k_strlen("!")) == 0) { //???
		wm_print (window_id, "\nyes !");
	} else if (k_memcmp (removed_command->buffer, "about", k_strlen("about")) == 0) {
		wm_print (window_id, "\nyes about");
	} else {
		wm_print (window_id, "\n[Error] Invalid command!");
	}
	
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
					wm_print (window_id, "\n[Error] Max command length is %d", MAX_COMMAND_LEN);
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


// For testing
void print_command (int window_id, Buffer_Command *command)
{
	wm_print (window_id, "\n[Test] You just typed: \n");
	for (int i = 0; i < command->length; i++) {
		wm_print (window_id, "%c", command->buffer[i]);
	}
	wm_print (window_id, "\n[Test] Command Length: %d\n", command->length);
}


void print_welcome (int window_id)
{
	wm_print (window_id, "****** Welcome to TOS Shell ******\n");
	wm_print (window_id, "Type [help] to show all TOS commands\n");
}


// Entry point of the shell process
void shell_process (PROCESS self, PARAM param)
{
	Buffer_Command command;
	Buffer_Command *command_ptr = &command;
	Buffer_Command *history_command[MAX_COMMAND_HISTORY];
	Buffer_Command removed_command;
	Buffer_Command *removed_command_ptr = &removed_command;
	int i = 0;
	BOOL exceed_limit; 

	int window_id = wm_create (10, 3, 60, 17);
	print_welcome (window_id);
	
	while (1) {
		wm_print (window_id, "\n> ");
		exceed_limit = read_command (window_id, command_ptr);
		if (exceed_limit == FALSE && command_ptr->buffer[0] != '\0') {
			history_command[i++] = command_ptr;
			//print_command (window_id, command_ptr); 

			remove_whitespace (command_ptr, removed_command_ptr);
			//print_command (window_id, removed_command_ptr); 

			execute_command (window_id, removed_command_ptr);	
			clear_command_buffer (removed_command_ptr);
		} 
		clear_command_buffer (command_ptr);
	}
}


// Init shell process
void start_shell ()
{
	create_process (shell_process, 5, 0, "Shell Porcess");
	resign ();
}
