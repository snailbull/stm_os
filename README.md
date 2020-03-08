# stm_os
stm_os is a event-driven library, use event to trig a state-machine.
It is a portable, cross-compilable, with a compatible API.
Use this to design ncurses game!

# framework
![](./doc/arch.png)

# how to use

	make
	./app.elf
	use keypad to play tetris.
>
	      up
	left down right
	p(pause)    q(quit)    space(hard drop)


# example
tetris game is a example for stm test.

stm is a mechanical Programming model, that's easy to design each state and transition action.


# 计划
1. 增加状态机栈，用于保存历史状态，然后再合适的事件下弹出历史状态
2. 增加状态队列，一次执行任务队列