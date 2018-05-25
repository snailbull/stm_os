#include "stm_os.h"
#include "fcmd.h"
#include <pthread.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <semaphore.h>

#include "driver_task.h"
#include "app_task.h"

/******************************************************************************
*
*/
#define MEM_POOL_SIZE   1024*40
uint8_t mem_pool[MEM_POOL_SIZE] __attribute__((aligned(4)));
TActiveCB activeCBs[] =
{
    {&driver_act, driver_queue, ARRAY_SIZE(driver_queue)},
    {&app_act, app_queue, ARRAY_SIZE(app_queue)},
};
uint8_t max_active_object = ARRAY_SIZE(activeCBs);


/******************************************************************************
*
*/
pthread_t dispatch_thread_id;
pthread_t timer_thread_id;
pthread_mutex_t os_mutex;
sem_t os_sem;

void *dispatch_thread(void *arg)
{
    driver_ctor();
    app_ctor();
	os_init();
	
    for (;;)
    {
        os_dispatch();
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

int main(int argc, char *argv[])
{
	char* input, shell_prompt[100];
	int res;

    // Configure readline to auto-complete paths when the tab key is hit.
    rl_bind_key('\t', rl_complete);

	// os init
    os_mem_init(mem_pool, mem_pool + MEM_POOL_SIZE - 1);
	
	// thread
    pthread_create(&dispatch_thread_id, NULL, dispatch_thread, NULL);
    pthread_create(&timer_thread_id, NULL, timer_thread, NULL);
    pthread_mutex_init(&os_mutex, NULL);
	res = sem_init(&os_sem, 0, 0);
    if(res == -1)
    {
		perror("os_sem intitialization failed\n");
		exit(EXIT_FAILURE);
    }

    for (;;)
    {
        // Create prompt string from user name and current working directory.
        snprintf(shell_prompt, sizeof(shell_prompt), "%s:%s $ ", getenv("USER"), getcwd(NULL, 1024));
  
        // Display prompt and read input (n.b. input must be freed after use)...
        input = readline(shell_prompt);
  
        // Check for EOF.
        if (!input)
            break;
  
        // Add input to history.
        add_history(input);
  
        // Do stuff...
		fcmd_exec(input);
  
        // Free input.
        free(input);
		input = NULL;
    }

    return 0;
}
