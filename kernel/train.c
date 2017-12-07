// Student Name:  Xinlu Chen  
// Student ID:    915757497

#include <kernel.h>

#define TRAIN_ID			"20"
#define TRAIN_CR			'\015'
#define TICK_SHORT			3
#define MAX_ARRAY_LEN		8
#define INPUT_BUFFER_LEN	3
#define PROBE_TIMES			3

#define CLOCKWISE			1
#define ANTI_CLOCKWISE		2

#define CONFIG_1			1
#define CONFIG_2			2
#define CONFIG_3			3
#define CONFIG_4			4
#define CONFIG_1_ZAMBONI	5
#define CONFIG_2_ZAMBONI	6
#define CONFIG_3_ZAMBONI	7
#define CONFIG_4_ZAMBONI	8


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
	
	// !!!Comment out to show a more clear view in the window area
	//wm_print (window_id, "s88 memory buffer cleaned (%s)\n", command);

	command[1] = TRAIN_CR;
	command[2] = '\0';
	send_command (ptr, &input_buffer, 0);
}


// Probe a contact to know if any vehicle is on it
char probe_contact (char* contact_id, int window_id)
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
	return input_buffer[1];
}


// Keep probing a certain contact unless it has some vehicle on it
void keep_probe (char* contact_id, int window_id)
{
	while (1)
	{
		if (probe_contact (contact_id, window_id) == '1') return;
		sleep (TICK_SHORT);
	}
}


// Change the direction of a train
void set_direction (int window_id)
{
	char input_buffer;
	char command[MAX_ARRAY_LEN];
	char *ptr = command;
	int len = k_strlen (TRAIN_ID);

	command[0] = 'L';
	k_memcpy (&command[1], TRAIN_ID, len);
	command[++len] = 'D';
	command[++len] = '\0';
	wm_print (window_id, "Reversed direction of train (%s)\n", command);
	
	command[len] = TRAIN_CR;
	command[++len] = '\0';
	send_command (ptr, &input_buffer, 0);
}


// Change the speed of a train
void set_speed (char speed, int window_id)
{
	char input_buffer;
	char command[MAX_ARRAY_LEN];
	char *ptr = command;
	int len = k_strlen (TRAIN_ID);

	command[0] = 'L';
	k_memcpy (&command[1], TRAIN_ID, len);
	command[++len] = 'S';
	command[++len] = speed;
	command[++len] = '\0';
	wm_print (window_id, "Changed train velocity to %c (%s)\n", speed, command);
	
	command[len] = TRAIN_CR;
	command[++len] = '\0';
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
	wm_print (window_id, "Changed switch %c to %c (%s)\n", switch_id, color, command);

	command[3] = TRAIN_CR;
	command[4] = '\0';
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


int detect_zamboni_direction (int window_id)
{
	wm_print (window_id, "Detecting the direction Zamboni is running... ");
	keep_probe ("4", window_id);
	for (int i = 0; i < 30; i++)
	{
		if (probe_contact ("6", window_id) == '1')
		{
			wm_print (window_id, "Done!\nZamboni is running clockwise\n");
			return CLOCKWISE;
		} 
		else if (probe_contact ("3", window_id) == '1')
		{
			wm_print (window_id, "Done!\nZamboni is running anti-clockwise\n");
			return ANTI_CLOCKWISE;
		}
		//sleep (TICK_SHORT);
	}
	wm_print (window_id, "Fail to detect the direction Zamboni is running\n");
	return -1;
}


// Check if Zamboni exists or not
BOOL detect_zamboni (int window_id)
{
	wm_print (window_id, "Detecting Zamboni... ");

	for (int i = 0; i < PROBE_TIMES; i++)
	{
		if (probe_contact ("4", window_id) == '1' || probe_contact ("6", window_id) == '1'
			|| probe_contact ("7", window_id) == '1' || probe_contact ("10", window_id) == '1'
			|| probe_contact ("13", window_id) == '1' || probe_contact ("14", window_id) == '1'
			|| probe_contact ("15", window_id) == '1' || probe_contact ("3", window_id) == '1')
		{
			wm_print (window_id, "Done!\nZamboni exists\n");
			return TRUE;
		} 	
	}
	wm_print (window_id, "Done!\nZamboni doesn't exist\n");
	return FALSE;
}


int locate_train_and_wagon (BOOL found_zamboni, int window_id)
{
	wm_print (window_id, "Locating Train and Wagon... Done!\n");
	if (probe_contact ("8", window_id) == '1' && probe_contact ("11", window_id) == '1')
	{
		if (found_zamboni) return CONFIG_1_ZAMBONI; 
		return CONFIG_1;
	} 
	else if (probe_contact ("12", window_id) == '1' && probe_contact ("2", window_id) == '1')
	{
		if (found_zamboni) return CONFIG_2_ZAMBONI; 
		return CONFIG_2;
	}
	else if (probe_contact ("5", window_id) == '1' && probe_contact ("11", window_id) == '1')
	{
		if (found_zamboni) return CONFIG_3_ZAMBONI; 
		return CONFIG_3;
	}
	else if (probe_contact ("16", window_id) == '1' && probe_contact ("9", window_id) == '1')
	{
		if (found_zamboni) return CONFIG_4_ZAMBONI; 
		return CONFIG_4;
	}
	else
	{
		wm_print (window_id, "[Error] Unknown configuration!\n");
		return -1;
	}
}


// Recognize which one of the eight configurations is chosen
int recognize_config (int window_id)
{
	BOOL found_zamboni;
	int config_num;
	int direction;

	wm_print (window_id, "Recognizing configurations...\n");
	found_zamboni = detect_zamboni (window_id);
	if (found_zamboni) 
	{
		direction = detect_zamboni_direction (window_id);
	}
	config_num = locate_train_and_wagon (found_zamboni, window_id);
	return config_num;
}


void run_config_1 (int window_id)
{
	wm_print (window_id, "Running configuration 1 without Zamboni\n");
	
	set_switch ('6', 'R', window_id);
	set_speed ('5', window_id);
	keep_probe ("7", window_id);
	set_speed ('0', window_id);
	set_direction (window_id);
	set_speed ('5', window_id);
	keep_probe ("14", window_id);
	set_switch ('1', 'R', window_id);
	set_switch ('2', 'R', window_id);
	set_switch ('7', 'R', window_id);
	set_switch ('8', 'R', window_id);
	keep_probe ("13", window_id);
	set_switch ('8', 'G', window_id);
	set_speed ('0', window_id);
	set_direction (window_id);
	set_speed ('5', window_id);
	keep_probe ("7", window_id);
	set_switch ('5', 'R', window_id);
	set_switch ('6', 'R', window_id);
	set_speed ('0', window_id);
	set_direction (window_id);
	set_speed ('5', window_id);
	keep_probe ("8", window_id);
	set_speed ('0', window_id);
	set_direction (window_id);
}


void run_config_2 (int window_id)
{
	wm_print (window_id, "Running configuration 2 without Zamboni\n");

	set_switch ('2', 'R', window_id);
	set_switch ('1', 'R', window_id);
	set_speed ('5', window_id);
	keep_probe ("14", window_id);
	set_speed ('0', window_id);
	set_direction (window_id);
	set_switch ('2', 'G', window_id);
	set_switch ('3', 'G', window_id);
	set_switch ('4', 'R', window_id);
	set_speed ('5', window_id);
	keep_probe ("6", window_id);
	set_speed ('0', window_id);
	set_direction (window_id);
	set_speed ('5', window_id);
	keep_probe ("14", window_id);
	set_speed ('0', window_id);
	set_direction (window_id);
	set_switch ('2', 'R', window_id);
	set_speed ('5', window_id);
	keep_probe ("12", window_id);
	set_speed ('0', window_id);
	set_direction (window_id);
}


void run_config_3 (int window_id)
{
	wm_print (window_id, "Running configuration 3 without Zamboni\n");

	set_switch ('3', 'R', window_id);
	set_switch ('4', 'R', window_id);
	set_speed ('5', window_id);
	set_switch ('1', 'R', window_id);
	set_switch ('2', 'R', window_id);
	set_switch ('7', 'R', window_id);
	keep_probe ("12", window_id);
	set_switch ('8', 'R', window_id);
	keep_probe ("14", window_id);
	set_speed ('0', window_id);
	set_direction (window_id);
	set_switch ('8', 'G', window_id);
	set_speed ('5', window_id);
	keep_probe ("5", window_id);
	set_speed ('0', window_id);
	set_direction (window_id);
}


void run_config_4 (int window_id)
{
	wm_print (window_id, "Running configuration 4 without Zamboni\n");

	set_switch ('9', 'G', window_id);
	set_speed ('5', window_id);
	keep_probe ("13", window_id);
	set_speed ('0', window_id);
	set_direction (window_id);
	set_switch ('9', 'R', window_id);
	set_switch ('1', 'R', window_id);
	set_switch ('2', 'R', window_id);
	set_switch ('7', 'G', window_id);
	set_switch ('6', 'G', window_id);
	set_switch ('5', 'R', window_id);
	set_speed ('5', window_id);

	keep_probe ("6", window_id);
	set_speed ('0', window_id);
	set_direction (window_id);
	set_switch ('5', 'G', window_id);
	set_switch ('9', 'G', window_id);
	set_speed ('5', window_id);
	keep_probe ("16", window_id);
	set_speed ('0', window_id);
	set_direction (window_id);
}


void run_config_1_zamboni (int window_id)
{
	wm_print (window_id, "Running configuration 1 with Zamboni\n");

	set_switch ('6', 'R', window_id);
	keep_probe ("10", window_id);   // Zamboni
	set_speed ('5', window_id);
	keep_probe ("7", window_id);  // sleep maybe after
	set_speed ('0', window_id);
	set_direction (window_id);
	set_speed ('5', window_id);
	keep_probe ("3", window_id);  // Zamboni maybe probe4
	set_switch ('1', 'R', window_id);
	set_switch ('2', 'R', window_id);
	set_switch ('7', 'R', window_id);
	keep_probe ("12", window_id);  // Train
	set_speed ('0', window_id);
	keep_probe ("13", window_id);  // Zamboni 
	set_switch ('8', 'R', window_id);
	set_speed ('5', window_id);
	keep_probe ("14", window_id);  // Zamboni
	keep_probe ("12", window_id);  // Zamboni sleep after probe12
	set_switch ('1', 'G', window_id);
	set_speed ('5', window_id);
	keep_probe ("3", window_id);   // Train and Wagon  sleep after probe3
	set_switch ('1', 'R', window_id);
	set_switch ('5', 'R', window_id);
	set_switch ('6', 'R', window_id);
	keep_probe ("8", window_id);
	set_speed ('0', window_id);
	set_direction (window_id);
}


void run_config_2_zamboni (int window_id)
{
	wm_print (window_id, "Running configuration 2 with Zamboni\n");
	
	set_switch ('2', 'R', window_id);
	keep_probe ("13", window_id);   // Zamboni
	set_switch ('1', 'R', window_id);
	set_speed ('5', window_id);
	keep_probe ("14", window_id);   // Train
	set_speed ('0', window_id);
	set_direction (window_id);
	set_switch ('2', 'G', window_id);
	set_speed ('5', window_id);
	keep_probe ("1", window_id);
	set_speed ('0', window_id);
	set_switch ('1', 'G', window_id);

	keep_probe ("4", window_id);	// Zamboni
	set_switch ('3', 'G', window_id);
	set_switch ('4', 'R', window_id);
	set_speed ('5', window_id);
	keep_probe ("6", window_id);    // Train
	set_speed ('0', window_id);
	set_direction (window_id);
	set_speed ('5', window_id);
	keep_probe ("1", window_id);	//Train
	set_speed ('0', window_id);
	set_switch ('4', 'G', window_id);

	keep_probe ("13", window_id);	// Zamboni	
	set_switch ('1', 'R', window_id);
	set_speed ('5', window_id);
	keep_probe ("14", window_id);	//Train
	set_speed ('0', window_id);
	set_direction (window_id);
	set_switch ('2', 'R', window_id);
	set_speed ('5', window_id);
	keep_probe ("12", window_id);
	set_speed ('0', window_id);
	set_direction (window_id);
	set_switch ('1', 'G', window_id);
}


void run_config_3_zamboni (int window_id)
{
	wm_print (window_id, "Running configuration 3 with Zamboni\n");

	set_switch ('3', 'R', window_id);
	keep_probe ("7", window_id);		// Zamboni
	set_switch ('4', 'R', window_id);
	set_speed ('5', window_id);
	
	keep_probe ("3", window_id);		// Zamboni
	set_switch ('4', 'G', window_id);
	set_switch ('1', 'R', window_id);
	set_switch ('2', 'R', window_id);
	set_switch ('7', 'R', window_id);
	
	keep_probe ("12", window_id);		//Train
	set_speed ('0', window_id);
	keep_probe ("14", window_id);		// Zamboni
	set_switch ('8', 'R', window_id);
	set_speed ('5', window_id);
	keep_probe ("13", window_id);		// Train
	set_switch ('1', 'G', window_id);
	keep_probe ("3", window_id);		//Train
	set_switch ('1', 'R', window_id);

	keep_probe ("7", window_id);		//Train
	set_speed ('0', window_id);
	set_direction (window_id);
	set_switch ('4', 'R', window_id);
	set_speed ('5', window_id);
	keep_probe ("5", window_id);		//Train
	set_speed ('0', window_id);
	set_direction (window_id);

}


void run_config_4_zamboni (int window_id)
{
	wm_print (window_id, "Running configuration 4 with Zamboni\n");

	keep_probe ("13", window_id);		// Zamboni
	set_switch ('9', 'G', window_id);
	set_speed ('5', window_id);
	
	keep_probe ("6", window_id);		// Zamboni
	set_switch ('9', 'R', window_id);
	set_switch ('2', 'R', window_id);
	set_switch ('7', 'G', window_id);
	set_switch ('6', 'G', window_id);
	keep_probe ("7", window_id);
	set_speed ('0', window_id);         // Train
	set_direction (window_id);
	set_switch ('5', 'R', window_id);
	set_speed ('5', window_id);
	keep_probe ("12", window_id);       // Train
	set_speed ('0', window_id);
	set_switch ('5', 'G', window_id);

	keep_probe ("10", window_id);		// Zamboni
	set_switch ('1', 'R', window_id);
	set_speed ('5', window_id);
	keep_probe ("13", window_id);		// Train
	set_speed ('0', window_id);
	set_direction (window_id);
	set_switch ('9', 'G', window_id);
	set_speed ('5', window_id);
	set_switch ('1', 'G', window_id);

	keep_probe ("16", window_id);		// Train
	set_speed ('0', window_id);
	set_direction (window_id);
	set_switch ('9', 'R', window_id);
}


void run_train (int window_id)
{
	int config = recognize_config (window_id);
	
	switch (config)
	{
		case CONFIG_1:
			run_config_1 (window_id);
			break;
		case CONFIG_2:
			run_config_2 (window_id);
			break;
		case CONFIG_3:
			run_config_3 (window_id);
			break;
		case CONFIG_4:
			run_config_4 (window_id);
			break;
		case CONFIG_1_ZAMBONI:
			run_config_1_zamboni (window_id);
			break;
		case CONFIG_2_ZAMBONI:
			run_config_2_zamboni (window_id);
			break;
		case CONFIG_3_ZAMBONI:
			run_config_3_zamboni (window_id);
			break;
		case CONFIG_4_ZAMBONI:
			run_config_4_zamboni (window_id);
			break;
		default:
			panic ("run_train(): wrong configuration");
	}
	wm_print (window_id, "Train and Wagon returned\n");
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

	/*
	probe_contact ("7", window_id);
	sleep (20);
	probe_contact ("10", window_id);
	sleep (100);
	probe_contact ("14", window_id);
	sleep (100);
	probe_contact ("3", window_id);
	*/
}


// Entry point of the train process
void train_process(PROCESS self, PARAM param)
{
	int window_id = wm_create (12, 5, 60, 17);
	wm_print (window_id, "****** Welcome to Train Application ******\n\n");
	init_switch (window_id);
	//test_train_command (window_id);
	run_train (window_id);
	while (1); // to avoid fatal exception 6
}


void init_train()
{
	create_process (train_process, 5, 0, "Train Process");
	resign ();
}
