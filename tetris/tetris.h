#ifndef _TETRIS_H_
#define _TETRIS_H_

#include "stm_os.h"

enum
{
    KEYBOARD_SIG = STM_SIG_USER,
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
    uint8_t type;       // block type:T L L` | Z Z` O
    uint8_t direction;  // rotate diretion
    uint8_t b[4][2];    // 1block have 4 piece(x,y)
}block_t;

typedef struct
{
    actor_t *act;
    int x,y;			// block origin
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

int director_init(stm_t *me, evt_t *e);
int block_init(stm_t *me, evt_t *e);
int field_init(stm_t *me, evt_t *e);
int score_init(stm_t *me, evt_t *e);
int stage_init(stm_t *me, evt_t *e);
int store_init(stm_t *me, evt_t *e);

#endif
