#include "stm_os.h"

static uint8_t rdy_grp;
static uint8_t rdy_tbl[CFG_RDY_TBL_SIZE];
static const uint8_t map_tbl[256] =
{
    0u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x00 to 0x0F */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x10 to 0x1F */
    5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x20 to 0x2F */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x30 to 0x3F */
    6u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x40 to 0x4F */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x50 to 0x5F */
    5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x60 to 0x6F */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x70 to 0x7F */
    7u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x80 to 0x8F */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x90 to 0x9F */
    5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xA0 to 0xAF */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xB0 to 0xBF */
    6u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xC0 to 0xCF */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xD0 to 0xDF */
    5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xE0 to 0xEF */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u  /* 0xF0 to 0xFF */
};
static uint8_t message_nesting_cnt;
//static uint8_t power_grp;
//static uint8_t power_tbl[CFG_RDY_TBL_SIZE];

/*
 * Initialize the activeCBs[]
 *
 * Note(s)  active object's prio is the index of activeCBs[]
 */
void os_init(void)
{
    TActive *act;
    TEvt evt;
    uint8_t i;

    for (i = 0; i < max_active_object; i++)
    {
        act = activeCBs[i].act;
        PORT_ASSERT(act != 0);

        act->prio   = i;
        act->head   = 0;
        act->tail   = 0;
        act->used   = 0;

        act->prio_x = i & 0x7;
        act->prio_y = i >> 3;
        act->prio_bit_y = (uint8_t) (1 << act->prio_y);
        act->prio_bit_x = (uint8_t) (1 << act->prio_x);
    }

    /* init state machine */
    evt.sig = STM_INIT_SIG;
    evt.para = 0;
    for (i = 0; i < max_active_object; i++)
    {
        act = activeCBs[i].act;
        act->super.vptr->init(&act->super, &evt);
    }
}

/*
 * Post event to specify active object
 *
 * @act   active object
 * @sig   signal num
 * @para  signal param
 * @opt   send to front or back of msg queue, SEND_TO_FRONT  SEND_TO_END
 *
 * Note(s) it might be called in interrupt. MESSAGE WILL PROCESSING ASYNCHRONOUS.
 */
uint8_t os_post_message(TActive *act, signal_t sig, void *para, uint8_t opt)
{
    TActiveCB *acb = &activeCBs[act->prio];
    PORT_SR_ALLOC();

    PORT_CPU_DISABLE();

    if (act->used >= acb->end)
    {
        PORT_CPU_ENABLE();
        return ERR_QUEUE_FULLED;
    }

    if (opt == SEND_TO_END)
    {
        acb->queue[act->tail].sig = sig;
        acb->queue[act->tail].para = para;

        act->tail++;
        if (act->tail >= acb->end)
        {
            act->tail = 0;
        }
    }
    else
    {
        if (act->head == 0)
        {
            act->head = acb->end;
        }
        act->head--;

        acb->queue[act->head].sig = sig;
        acb->queue[act->head].para = para;
    }

    act->used++;
    if (act->used == 1)
    {
        rdy_grp |= acb->act->prio_bit_y;
        rdy_tbl[acb->act->prio_y] |= acb->act->prio_bit_x;
    }

    PORT_CPU_ENABLE();
	os_wakeup();

    return ERR_SUCCESS;
}

/*
 * Send event to specify active object
 *
 * @act   active object
 * @sig   signal num
 * @para  signal param
 *
 * Note(s) it might be called in interrupt. MESSAGE WILL PROCESSING SYNCHRONOUS.
 */
uint8_t os_send_message(TActive *act, signal_t sig, void *para)
{
    TEvt evt = {sig, para};
    message_nesting_cnt++;	/* ++ and -- always in pairs */
    if (message_nesting_cnt > CFG_MSG_NEST_DEPTH)
    {
        if (message_nesting_cnt)
        {
            message_nesting_cnt--;
        }
        return ERR_NESTING_FULLED;
    }

    /* recursive dispatch */
    act->super.vptr->dispatch(&act->super, &evt);

    if (message_nesting_cnt)
    {
        message_nesting_cnt--;
    }
    return ERR_SUCCESS;
}

void os_dispatch(void)
{
    TActiveCB *acb;
    TActive *act;
    TEvt evt;
    uint8_t y;
    uint8_t prio_highest_rdy;
    uint8_t i;
    PORT_SR_ALLOC();

    PORT_CPU_DISABLE();

    if (rdy_grp)                /* anyone active object get events. */
    {
        /* find the highest ready active object and activeCB. */
        y = map_tbl[rdy_grp];
        prio_highest_rdy = ( (y << 3) + map_tbl[rdy_tbl[y]]);

        acb = &activeCBs[prio_highest_rdy];
        act = activeCBs[prio_highest_rdy].act;

        /* pick a event from queue head. */
        act->used--;
        if (act->used == 0) /* no event in queue, clear active object's rdy bit */
        {
            rdy_tbl[act->prio_y] &= ~act->prio_bit_x;
            if (rdy_tbl[act->prio_y] == 0)
            {
                rdy_grp &= ~act->prio_bit_y;
            }
        }

        evt.sig = acb->queue[act->head].sig;
        evt.para = acb->queue[act->head].para;
        act->head++;
        if (act->head >= acb->end)
        {
            act->head = 0;
        }

        PORT_CPU_ENABLE();

        /* process this event */
        act->super.vptr->dispatch(&act->super, &evt);
    }
    else
    {
        PORT_CPU_ENABLE();
        /* power manegement */
		os_sleep();
    }
}

void os_sleep(void)
{
#if CFG_POWER_SAVING > 0u
	PORT_OS_SLEEP();
#endif
}
void os_wakeup(void)
{
#if CFG_POWER_SAVING > 0u
	PORT_OS_WKUP();
#endif
}
/*
 * Find the highest ready task
 * i = bit_search_first_one(rq->task_bit_map, priority, CFG_LOWEST_PRIO - priority);
 */
int32_t bit_search_first_one(int32_t *base, uint8_t offset, int32_t width)
{
    register uint32_t *cp, v;
    register int32_t position;

    cp = (uint32_t*)base;
    /*caculate word position to bitmap*/
    cp += offset >> 5;

    /* clear all bit before offset(not include offset bit), do not need do this if offset is 32, 64, 96, etc......*/
    if (offset & 31)
    {
#if (CFG_LITTLE_ENDIAN > 0)
        v = *cp & ~(((uint32_t)1 << (offset & 31)) - 1);
#else
        v = *cp & (((uint32_t)1 << (32 - (offset & 31))) - 1);
#endif
    }
    else
    {
        v = *cp;
    }

    position = 0;
    while (position < width)
    {
        if (v)
        {
            /*Set right position first time*/
            if (!position)
            {
                position -= (offset & 31);
            }
#if  (CFG_LITTLE_ENDIAN > 0)
            if (!(v & 0xffff))
            {
                v >>= 16;
                position += 16;
            }
            if (!(v & 0xff))
            {
                v >>= 8;
                position += 8;
            }
            if (!(v & 0xf))
            {
                v >>= 4;
                position += 4;
            }
            if (!(v & 0x3))
            {
                v >>= 2;
                position += 2;
            }
            if (!(v & 0x1))
            {
                ++position;
            }
#else
            if (!(v & 0xffff0000))
            {
                v <<= 16;
                position += 16;
            }
            if (!(v & 0xff000000))
            {
                v <<= 8;
                position += 8;
            }
            if (!(v & 0xf0000000))
            {
                v <<= 4;
                position += 4;
            }
            if (!(v & 0xc0000000))
            {
                v <<= 2;
                position += 2;
            }
            if (!(v & 0x80000000))
            {
                ++position;
            }
#endif
            if (position < width)
            {
                return position;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            /*Skip one world*/
            if (position)
            {
                position += 32;
            }
            else
            {
                position = 32 - (offset & 31);
            }
            v = *++cp;
        }
    }
    return -1;
}
