/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* b2_socket.h */


#ifndef B2_SOCKET_H
#define B2_SOCKET_H

/* returns socket descriptor */
int b2_create_accept_socket(const unsigned short port);

/* returns socket descriptor */
int b2_create_connect_socket(const char* hostname, const unsigned short port);

/* timeout>0 ... timed I/O (secs), time==0 ... non-blocking I/O, time<0 ... blocking I/O */
int b2_timed_blocking_io(const int sd, const int timeout /* sec */);

int b2_build_sockaddr_in(const char* hostname, const unsigned short port, struct sockaddr_in* in);

int b2_send(const int sd, const void* buf, const size_t size);
int b2_recv(const int sd,       void* buf, const size_t size);


#endif /* B2_SOCKET_H */

