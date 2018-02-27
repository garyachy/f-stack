#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>

#include "ff_config.h"
#include "ff_api.h"
#include <unistd.h>

#define MAX_EVENTS 512

/* kevent set */
struct kevent kevSet;
/* events */
struct kevent events[MAX_EVENTS];
/* kq */
int kq;
int sockfd;

static char buff[100] = "";
static uint32_t counter = 0;

static unsigned long long num_bytes = 0;
static struct timeval  tv1, tv2;

int send_loop()
{
    int ret = 0;

    if (num_bytes == 0)
            {
                gettimeofday(&tv1, NULL);
            }


    ret = snprintf(buff, sizeof(buff), "%u,",  counter);
    ff_write(sockfd, buff, ret);
    num_bytes += ret; 
    counter++;

            if (num_bytes == 1288890)
            {
                gettimeofday(&tv2, NULL);
                float secs = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
                printf("Sent %u messages, %u bytes, %f sec, %f B/s, %f Msg/sec \n",
                       counter, num_bytes, secs, num_bytes/secs, counter/secs);
            }


}

static unsigned long cur_count = 0;
#define MAX_COUNT 200000

int loop(void *arg)
{
    /* Wait for events to happen */
  unsigned nevents = ff_kevent(kq, NULL, 0, events, MAX_EVENTS, NULL);

  if (cur_count < MAX_COUNT)
  {
    send_loop();
    cur_count++;
  }
}

int main(int argc, char * argv[])
{
  // init DPDK context and FreeBSD network stack
  ff_init(argc, argv);

  // create socket fd
  sockfd = ff_socket(AF_INET, SOCK_STREAM, 0);
  printf("sockfd:%d\n", sockfd);
  if (sockfd < 0)
    printf("ff_socket failed\n");

      struct sockaddr_in remoteAddr;
      int nAddrLen = sizeof(remoteAddr);

    remoteAddr.sin_family = AF_INET;
remoteAddr.sin_addr.s_addr = inet_addr("172.31.18.158");
    remoteAddr.sin_port = htons(5000);

    ff_connect(sockfd, (struct linux_sockaddr *)&remoteAddr, nAddrLen);

    kevSet.data     = MAX_EVENTS;
    kevSet.fflags   = 0;
    kevSet.filter   = EVFILT_READ;
    kevSet.flags    = EV_ADD;
    kevSet.ident    = sockfd;
    kevSet.udata    = NULL;

    assert((kq = ff_kqueue()) > 0);

    /* Update kqueue */
    ff_kevent(kq, &kevSet, 1, NULL, 0, NULL);

    // the loop function would be called by each poll of f-stack
    ff_run(loop, NULL);
    return 0;
}
