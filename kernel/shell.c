// Student Name:  Xinlu Chen  
// Student ID:    915757497

#include <kernel.h>

#define MAX_COMMAND_LEN 30
#define MAX_COMMAND_HISTORY 100

void shell_process (PROCESS self, PARAM param);

typedef struct _Buffer_Command
{
	char buffer[MAX_COMMAND_LEN];
	int length;
} Buffer_Command;


void help (int window_id)
{
	wm_print (window_id, "TOS commands: \n");
	wm_print (window_id, "help      print all supported TOS commands\n");
	wm_print (window_id, "cls       clear the screen/window\n");
	wm_print (window_id, "shell     launch another shell\n");
	wm_print (window_id, "pong      launch the PONG game\n");
	wm_print (window_id, "ps        print out the process table\n");
	wm_print (window_id, "history   print all commands that have beeen typed\n");
	wm_print (window_id, "!<number> repeat the command with the given number\n");
	wm_print (window_id, "about     print out developer's name\n");
	wm_print (window_id, "train     launch train application\n");
}


void print_details (int window_id, PCB *ptr)
{
	static const char *state[] =
	{ "READY           ",
	  "SEND_BLOCKED    ",
	  "REPLY_BLOCKED   ",
	  "RECEIVE_BLOCKED ",
	  "MESSAGE_BLOCKED ",
	  "INTR_BLOCKED    "
	};
	
	if (!ptr->used) {
		wm_print (window_id, "[Error] PCB slot unused\n");
		return;
	}

	wm_print (window_id, state[ptr->state]);
	if (ptr == active_proc) {
		wm_print (window_id, " *     ");
	} else {
		wm_print (window_id, "       ");
	}
	wm_print (window_id, " %2d", ptr->priority);
	wm_print (window_id, "      %s\n", ptr->name);
}


void print_processes (int window_id)
{
	wm_print (window_id, "State           Active   Prio   Name\n");
	wm_print (window_id, "-----------------------------------------------------\n");
	PCB *ptr = pcb;
	for (int i = 0; i < MAX_PROCS; i++, ptr++) {
		if (!ptr->used) continue;
		print_details (window_id, ptr);
	}
}


void show_history (int window_id, Buffer_Command *history_command_ptr, int *num)
{
	for (int i = 0; i <= *num; i++) {
		wm_print (window_id, "%2d %s\n", i, (*history_command_ptr).buffer);
		history_command_ptr++;
	}
}


int get_history_index (Buffer_Command *removed_command)
{
	int history_index = 0;
	int digit = 0;
	for (int k = 1; k < removed_command->length; k++) {
		char c = removed_command->buffer[k];
		if (c < '0' || c > '9') return -1;
		digit = c - '0';    // transfer from char to int
		history_index = history_index * 10 + digit;
	}
	return history_index;
}


BOOL is_valid_repeat_command (int window_id, Buffer_Command *removed_command, int *num)
{
	if (removed_command->buffer[0] != '!' || removed_command->length > 3) { // history_index 0-99
		return FALSE;
	}

	int history_index = get_history_index (removed_command);
	//wm_print (window_id, "history_index: %d\n", history_index);

	if (history_index > *num || history_index < 0) {
		return FALSE;
	} 
	return TRUE;
}


void repeat_command (int window_id, Buffer_Command *removed_command, Buffer_Command *history_command_ptr, int *num)
{
	int history_index = get_history_index (removed_command);
	Buffer_Command command = *(history_command_ptr + history_index);
	wm_print (window_id, "%s\n", command.buffer);
	process_command (window_id, &command, removed_command, history_command_ptr, num);
}


void about (int window_id)
{
	wm_print (window_id, "----------- Peace and Love -----------\n\n");
	wm_print (window_id, "              Xinlu Chen                \n");
	wm_print (window_id, "        =^__________________^=          \n");
}


BOOL compare_string (Buffer_Command *removed_command, char *target)
{
	int length = k_strlen (target);
	BOOL is_same_char = (k_memcmp (removed_command->buffer, target, length) == 0);
	BOOL is_same_length = (removed_command->length == length);
	return is_same_char && is_same_length;
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


void execute_command (int window_id, Buffer_Command *removed_command, Buffer_Command *history_command_ptr, int *num)
{
	if (compare_string (removed_command, "help")) {
		help (window_id);

	} else if (compare_string (removed_command, "cls")) {
		wm_clear (window_id);

	} else if (compare_string (removed_command, "shell")) {
		create_process (shell_process, 5, 0, "Shell Porcess");

	} else if (compare_string (removed_command, "pong")) {
		start_pong ();

	} else if (compare_string (removed_command, "ps")) {
		print_processes (window_id);

	} else if (compare_string (removed_command, "history")) {
		show_history (window_id, history_command_ptr, num);

	} else if (is_valid_repeat_command (window_id, removed_command, num)) {
		repeat_command (window_id, removed_command, history_command_ptr, num);

	} else if (compare_string (removed_command, "about")) {
		about (window_id);

	} else if (compare_string (removed_command, "train")) {
		init_train ();

	} else {
		wm_print (window_id, "[Error] Invalid command!\n");
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
					wm_print (window_id, "\n[Error] Max command length is %d\n", MAX_COMMAND_LEN);
					return exceed_limit;
				} else {
					command->buffer[command->length++] = keystroke;
					wm_print (window_id, "%c", keystroke);
				}
				break;
		}
		keystroke = keyb_get_keystroke (window_id, TRUE);
	}

	command->buffer[command->length] = '\0';
	wm_print (window_id, "\n");
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
	wm_print (window_id, "[Test] You just typed: \n");
	for (int i = 0; i < command->length; i++) {
		wm_print (window_id, "%c", command->buffer[i]);
	}
	wm_print (window_id, "\n[Test] Command Length: %d\n", command->length);
}


void print_welcome (int window_id)
{
	wm_print (window_id, "****** Welcome to TOS Shell ******\n");
	wm_print (window_id, "Type [help] to show all TOS commands\n\n");
}


void process_command (int window_id, Buffer_Command *command, Buffer_Command *removed_command, Buffer_Command *history_command, int *i)
{
	remove_whitespace (command, removed_command);
	//print_command (window_id, removed_command); 

	// Currently: only store in history table when i < MAX_COMMAND_HISTORY
	if (removed_command->buffer[0] != '!' && *i + 1 < MAX_COMMAND_HISTORY) {
		//wm_print (window_id, "history: \n%2d %s\n", i, removed_command->buffer);
		history_command[++(*i)] = *command;
	}

	execute_command (window_id, removed_command, history_command, i);
	//clear_command_buffer (removed_command);	
}


// Entry point of the shell process
void shell_process (PROCESS self, PARAM param)
{
	Buffer_Command command;
	Buffer_Command removed_command;
	Buffer_Command history_command[MAX_COMMAND_HISTORY];
	int i = -1;
	BOOL exceed_limit; 

	int window_id = wm_create (10, 3, 55, 17);
	print_welcome (window_id);
	
	while (1) {
		wm_print (window_id, "> ");
		exceed_limit = read_command (window_id, &command);
		if (exceed_limit == FALSE && command.buffer[0] != '\0') {
			process_command (window_id, &command, &removed_command, &history_command, &i);
		} 
		clear_command_buffer (&command);
	}
}


// Init shell process
void start_shell ()
{
	create_process (shell_process, 5, 0, "Shell Porcess");
	resign ();
}
