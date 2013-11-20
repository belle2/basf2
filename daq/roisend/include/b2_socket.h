/* b2_socket.h */


#ifndef B2_SOCKET_H
#define B2_SOCKET_H


#include <netinet/in.h>


/* returns socket descriptor */
extern const int b2_create_accept_socket(const unsigned short port);

/* returns socket descriptor */
extern const int b2_create_connect_socket(const char* hostname, const unsigned short port);

/* timeout>0 ... timed I/O (secs), time==0 ... non-blocking I/O, time<0 ... blocking I/O */
extern const int b2_timed_blocking_io(const int sd, const int timeout /* sec */);

const int b2_build_sockaddr_in(const char* hostname, const unsigned short port, struct sockaddr_in* in);

extern const int b2_send(const int sd, const void* buf, const size_t size);
extern const int b2_recv(const int sd,       void* buf, const size_t size);


#endif /* B2_SOCKET_H */

