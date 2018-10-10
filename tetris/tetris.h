#ifndef _TETRIS_H_
#define _TETRIS_H_

#include "stm_os.h"


enum
{
    KEY_SIG = STM_EVT_USER,
};


typedef struct
{
    actor_t *act;
    int x,y;
    int width,height;
    uint8_t b[20][10];
}field_act_t;

typedef struct
{
    uint8_t type;       // block type:T L | Z O
    uint8_t direction;  // rotate diretion
    uint8_t b[4][2];    // 1block have 4 piece(x,y)
}block_t;

typedef struct
{
    actor_t *act;
    int x,y;
    block_t b;
    uint8_t speed;
}block_act_t;

typedef struct
{
    actor_t *act;
    int x,y;
    block_t b[2];       // next block
}store_act_t;

typedef struct
{
    actor_t *act;
    int x,y;
    int score;
    uint8_t level;
    int lines;
}score_act_t;

typedef struct
{
    actor_t *act;
    int x,y;
    int width, height;
    uint8_t b[25][40];
}stage_act_t;

extern actor_t *director_act;


uint8_t director_init(stm_t *me, msg_t *e);
uint8_t block_init(stm_t *me, msg_t *e);
uint8_t field_init(stm_t *me, msg_t *e);
uint8_t score_init(stm_t *me, msg_t *e);
uint8_t stage_init(stm_t *me, msg_t *e);
uint8_t store_init(stm_t *me, msg_t *e);

#endif
