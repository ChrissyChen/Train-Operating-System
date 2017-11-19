// Student Name:  Xinlu Chen  
// Student ID:    915757497

#include <kernel.h>


void shell_process (PROCESS self, PARAM param)
{
	int window_id = wm_create (10, 3, 50, 17);
	wm_print (window_id, "Hello World!!!\n");
	
	while (1) {
		char ch = keyb_get_keystroke (window_id, TRUE);
		wm_print (window_id, "Got key: %c\n", ch);
	}
}






// init shell process
void start_shell ()
{
	create_process (shell_process, 5, 0, "Shell Porcess");
	resign ();
}
