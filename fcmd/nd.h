#ifndef __ND_H__
#define __ND_H__

int nd_init(int port);
int nd_exit(void);
int nd_send(char *s, int len);
int nd_printf(const char *format, ...);

#endif
