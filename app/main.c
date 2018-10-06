#include "stm_os.h"
#include "fcmd.h"
#include <pthread.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <curses.h>

#include "driver_task.h"
#include "app_task.h"

#define MEM_POOL_SIZE   1024*40
uint8_t mem_pool[MEM_POOL_SIZE];

pthread_t dispatch_thread_id;
pthread_t timer_thread_id;
pthread_t scr_thread_id;

actor_t *app_act;
actor_t *driver_act;

void *dispatch_thread(void *arg)
{
    app_act = actor_add((stm_func_t)app_init, 10, 1);
    driver_act = actor_add((stm_func_t)driver_init, 10, 1);
	
    for (;;)
    {
        actor_dispatch();
    }
}

void *timer_thread(void *arg)
{
    for (;;)
    {
        evtimer_update(1);
        cbtimer_update(1);
        usleep(1000);
    }
}

void *scr_thread(void *arg)
{
    initscr();      /* initialize the curses library */
    if (has_colors())
    {
        start_color();
        init_pair(1, COLOR_RED,     COLOR_BLACK);
        init_pair(2, COLOR_GREEN,   COLOR_BLACK);
        init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(4, COLOR_BLUE,    COLOR_BLACK);
        init_pair(5, COLOR_CYAN,    COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_WHITE,   COLOR_BLACK);
    }
    
    for (;;)
    {
        int c = getch();     /* refresh, accept single keystroke of input */

        /* process the command keystroke */
        if (c == 'q' || c == 'Q')
        {
            mvprintw(10,0,"Shelling out...");
            def_prog_mode();           /* save current tty modes */
            endwin();                  /* restore original tty modes */
            system("bash");              /* run shell */
            mvprintw(10,0,"returned.      ");     /* prepare return message */
            refresh();                 /* restore save modes, repaint screen */
        }
    }
}
int main(int argc, char *argv[])
{
	char* input, shell_prompt[100];
	int res;

    // Configure readline to auto-complete paths when the tab key is hit.
    // rl_bind_key('\t', rl_complete);

	// os init
    os_mem_init(mem_pool, mem_pool + MEM_POOL_SIZE - 1);
	
	// thread
    pthread_create(&dispatch_thread_id, NULL, dispatch_thread, NULL);
    pthread_create(&timer_thread_id, NULL, timer_thread, NULL);
    pthread_create(&scr_thread_id, NULL, scr_thread, NULL);

    for (;;)
    {
        sleep(5000);
        // // Create prompt string from user name and current working directory.
        // snprintf(shell_prompt, sizeof(shell_prompt), "%s:%s $ ", getenv("USER"), getcwd(NULL, 1024));
  
        // // Display prompt and read input (n.b. input must be freed after use)...
        // input = readline(shell_prompt);
  
        // // Check for EOF.
        // if (!input)
        //     break;
  
        // // Add input to history.
        // add_history(input);
  
        // // Do stuff...
		// fcmd_exec(input);
  
        // // Free input.
        // free(input);
		// input = NULL;
    }

    return 0;
}
