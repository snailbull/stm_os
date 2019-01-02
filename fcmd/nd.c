#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdarg.h>

#include "fcmd.h"
#include "nd.h"

enum
{
    ND_CLOSE=0,
    ND_BEGIN,
    ND_CONNECT,
};

static int nd_sock;
static pthread_t nd_thread_id;

static struct sockaddr_in nd_local_addr;
static struct sockaddr_in nd_remote_addr;

#define ND_SEND_BUF_LEN 4096
static char nd_send_buf[ND_SEND_BUF_LEN];
static int nd_send_len;

static char nd_recv_buf[256];
static char nd_state;

static int s_nd_send_tim;

void nd_send_cb(void);

void *nd_thread(void *arg)
{
    int rb;
    struct timeval timeout;
    fd_set fdset;
    int addrlen = sizeof(nd_remote_addr);

    nd_state = ND_CONNECT;

    while(1)
    {
        FD_ZERO(&fdset);
        FD_SET(nd_sock, &fdset);
        timeout.tv_sec = 0;
        timeout.tv_usec = 50 * 1000;// 50ms
        if ( (select(nd_sock + 1, &fdset, NULL, NULL, &timeout) > 0) &&
             (FD_ISSET(nd_sock, &fdset)))
        {
            {
                rb = recvfrom(nd_sock, nd_recv_buf,sizeof(nd_recv_buf),0, (struct sockaddr*)&nd_remote_addr, &addrlen);
                if ((rb >= 3) && (nd_recv_buf[0] == '#') && (nd_recv_buf[rb-1] == '@'))
                {
                    // send to this socket, remote_addr:00000000, remote_port:36895
                    //printf("remote_addr:%08x, remote_port:%d\n", nd_remote_addr.sin_addr.s_addr, nd_remote_addr.sin_port);
                    nd_recv_buf[rb-1] = 0;
                    fcmd_exec(&nd_recv_buf[1]);
                }
            }
        }
        
        // nd_send timer
        if (s_nd_send_tim > 0)
        {
            s_nd_send_tim--;
        }
        else if (s_nd_send_tim == 0)
        {
            s_nd_send_tim = -1;
            nd_send_cb();
        }
        else
        {
            ;
        }

        // exit signal
        if (nd_state == ND_CLOSE)
        {
            close(nd_sock);
            pthread_exit(NULL);// This function does not return to the caller.
        }
    }
}

// create udp debug
int nd_send(char *d, int len)
{
    if (nd_state == ND_CONNECT)
    {
        return sendto(nd_sock, d, len, 0, (struct sockaddr*)&nd_remote_addr, sizeof(struct sockaddr));
    }
    return -1;
}

void nd_send_cb(void)
{
	if (nd_send_len)
	{
		nd_send(nd_send_buf, nd_send_len);
		nd_send_len = 0;
	}
}

int nd_init(int port)
{
    if (nd_state > ND_CLOSE)
    {
        return -1;
    }
    nd_state = ND_BEGIN;

    nd_sock = socket(AF_INET,SOCK_DGRAM,0);
    if (nd_sock < 0)
    {
        nd_state = ND_CLOSE;
        return -1;
    }
    
    int opt = -1;
    if (setsockopt(nd_sock, SOL_SOCKET, SO_BROADCAST, (void*)&opt, sizeof(opt)) < 0)
    {
        nd_state = ND_CLOSE;
        close(nd_sock);
        return -1;
    }

    // local
    bzero(&nd_local_addr, sizeof(struct sockaddr_in));
    nd_local_addr.sin_family      = AF_INET;
    nd_local_addr.sin_port        = htons(port);
    nd_local_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    if (0 > bind(nd_sock, (struct sockaddr*)&nd_local_addr, sizeof(nd_local_addr)))
    {
        nd_state = ND_CLOSE;
        close(nd_sock);
        return -1;
    }

    // remote
    bzero(&nd_remote_addr, sizeof(struct sockaddr_in));
    nd_remote_addr.sin_family      = AF_INET;
    nd_remote_addr.sin_port        = htons(50000);
    nd_remote_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    
    pthread_create(&nd_thread_id, NULL, nd_thread, NULL);
    return 0;
}

int nd_exit(void)
{
    if (nd_state < ND_CONNECT)
    {
        return -1;
    }
    nd_state = ND_CLOSE;
    pthread_join(&nd_thread_id, NULL);  // wait pthread exit, and free pthread source.
    return 0;
}

int nd_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int n = vsnprintf(nd_send_buf+nd_send_len, ND_SEND_BUF_LEN-nd_send_len, format, args);
    va_end (args);

    nd_send_len += n;
	if (nd_send_len > ND_SEND_BUF_LEN)
	{
		nd_send_len = ND_SEND_BUF_LEN;
	}
    s_nd_send_tim = 3;  // 150ms interval.

    return n;
}
