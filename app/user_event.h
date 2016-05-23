#ifndef _USER_EVENT_H_
#define _USER_EVENT_H_

#include "stm_os.h"

enum
{
    KEY_SIG = STM_USER_SIG,
    ADC_SIG,
    LED_SIG,
    FLUSH_SIG,
    APP_NET_SIG,
};

#endif
