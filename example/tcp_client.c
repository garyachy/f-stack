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

#define MAX_EVENTS 512

/* kevent set */
struct kevent kevSet;
/* events */
struct kevent events[MAX_EVENTS];
/* kq */
int kq;
int sockfd;

char html[] =
  "HTTP/1.1 200 OK\r\n"
  "Server: F-Stack\r\n"
  "Date: Sat, 25 Feb 2017 09:26:33 GMT\r\n"
  "Content-Type: text/html\r\n"
  "Content-Length: 439\r\n"
  "Last-Modified: Tue, 21 Feb 2017 09:44:03 GMT\r\n"
  "Connection: keep-alive\r\n"
  "Accept-Ranges: bytes\r\n"
  "\r\n"
  "<!DOCTYPE html>\r\n"
  "<html>\r\n"
  "<head>\r\n"
  "<title>Welcome to F-Stack!</title>\r\n"
  "<style>\r\n"
  "    body {  \r\n"
  "        width: 35em;\r\n"
  "        margin: 0 auto; \r\n"
  "        font-family: Tahoma, Verdana, Arial, sans-serif;\r\n"
  "    }\r\n"
  "</style>\r\n"
  "</head>\r\n"
  "<body>\r\n"
  "<h1>Welcome to F-Stack!</h1>\r\n"
  "\r\n"
  "<p>For online documentation and support please refer to\r\n"
  "<a href=\"http://F-Stack.org/\">F-Stack.org</a>.<br/>\r\n"
  "\r\n"
  "<p><em>Thank you for using F-Stack.</em></p>\r\n"
  "</body>\r\n"
  "</html>";

int loop(void *arg)
{
  /* Wait for events to happen */
  unsigned nevents = ff_kevent(kq, NULL, 0, events, MAX_EVENTS, NULL);
//  printf("send %u bytes\n", sizeof(html));

      ff_write(sockfd, html, sizeof(html));

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
