#include "stm_os.h"
#include "fcmd.h"
#include <pthread.h>
#include <unistd.h>

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
    uint8_t *buf = malloc(1024);

    os_mem_init(mem_pool, mem_pool + MEM_POOL_SIZE - 1);

    pthread_create(&dispatch_thread_id, NULL, dispatch_thread, NULL);
    pthread_create(&timer_thread_id, NULL, timer_thread, NULL);
    pthread_mutex_init(&os_mutex, NULL);

    for (;;)
    {
        gets(buf);
        fcmd_exec(buf);
    }

    free(buf);

    return 0;
}
