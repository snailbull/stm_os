#include "stm_os.h"
#include "fcmd.h"
#include "nd.h"

#include <pthread.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <curses.h>

#include "tetris.h"

#define MEM_POOL_SIZE   1024*40
static uint8_t mem_pool[MEM_POOL_SIZE];
pthread_t scheduler_thread_id;
pthread_t timer_thread_id;
pthread_t device_thread_id;

// actor scheduler
void *scheduler_thread(void *arg)
{
    // must add director!
    director_act = actor_create(director_init, 10, 1);
    for (;;)
    {
        actor_dispatch();
    }
}

// timer event
void *timer_thread(void *arg)
{
    for (;;)
    {
        evtimer_update(1);
        cbtimer_update(1);
        usleep(1000);
    }
}

// screen output & keypad input
void *device_thread(void *arg)
{
    initscr();
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
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    clear();
    
    for (;;)
    {
        int c = getch();     /* refresh, accept single keystroke of input */
        switch(c)
        {
            case KEY_UP:
            break;

            case KEY_DOWN:
            break;

            case KEY_LEFT:
            break;

            case KEY_RIGHT:
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    os_mem_init(mem_pool, mem_pool + MEM_POOL_SIZE - 1);
    nd_init(50791);
    pthread_create(&scheduler_thread_id, NULL, scheduler_thread, NULL);
    pthread_create(&timer_thread_id, NULL, timer_thread, NULL);
    //pthread_create(&device_thread_id, NULL, device_thread, NULL);

    for (;;)
    {
        sleep(5000);
    }

    return 0;
}
