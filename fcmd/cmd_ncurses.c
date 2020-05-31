#include <curses.h>

void test_win(void)
{
	int h,w,y,x;
	WINDOW *win_log;

	h = 10; w = 25;
	y = 10; x = 30;
	win_log = newwin(h, w, y, x);
	box(win_log, 0,0);
	mvwprintw(win_log, 1,1, "this is log window");
	wrefresh(win_log);
}
void test_screen_size(void)
{
	int x,y;
	int x0,y0;
	int w,h;
	mvaddch(7,15, 'C');
	getyx(stdscr, y, x);
	getbegyx(stdscr, y0, x0);
	getmaxyx(stdscr, h, w);
	attron(COLOR_PAIR(2));
	mvprintw(10,10, "Origin:(%d,%d), Cur:(%d,%d), Full:(%d,%d)",
					y0,x0, y,x, h,w);
	attroff(COLOR_PAIR(2));
	refresh();
}
void test_user_input(void)
{
	int h,w,c;
	WINDOW *input;

	getmaxyx(stdscr, h, w);
	input = newwin(3, w-6, h-5, 3);
	box(input, 0,0);
	mvwprintw(input, 1,1, "this is input window");
	keypad(input, true);

	for (;;)
	{
		c = wgetch(input);
		waddch(input, c);
		wrefresh(input);
	}
}
void test_menu_select(void)
{
	int h,w,c;
	WINDOW *input;

	getmaxyx(stdscr, h, w);
	input = newwin(3, w-6, h-5, 3);
	box(input, 0,0);
	keypad(input, true);

	char *choice[3]={"Walk","Run","Sleep"};
	int items = sizeof(choice)/sizeof(choice[0]);
	int highlight = 0;
	int i;
	for (;;)
	{
		for (i=0; i<items; i++) {
			if (i == highlight)
				wattron(input, A_REVERSE);
			mvwprintw(input, 1, 1+i*10, choice[i]);
			wattroff(input, A_REVERSE);
		}

		c = wgetch(input);
		switch (c) {
			case KEY_LEFT:
				highlight = (highlight+items-1)%items;
				break;
			case KEY_RIGHT:
				highlight = (highlight+items+1)%items;
				break;
		}
	}
}
void test_input_mode(void)
{
	int h,w,c;
	WINDOW *input;

	getmaxyx(stdscr, h, w);
	input = newwin(3, w-6, h-5, 3);
	box(input, 0,0);
	keypad(input, true);
	wtimeout(input, 500);
	// nodelay(input, true);
	for (;;)
	{
		c = wgetch(input);
		wprintw(input, "%c", c);
	}
}
void test_curses(void)
{
	// test_win();
	// test_screen_size();
	// test_user_input();
	// test_menu_select();
	test_input_mode();
}
