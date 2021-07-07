/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/dataflow/SocketLib.h>
#include <daq/dataflow/SocketManager.h>

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  // Open socket to accept connection
  SocketRecv recsock((unsigned short)(atoi(argv[1])));

  // Register it in SocketManager
  SocketManager sockman(recsock.sock());

  // Loop forever
  int count = 0;
  for (;;) {
    // Check I/O request
    int is = sockman.examine();
    if (is == 0) {
      printf("New connection made\n");
    } else if (is == 1) {
      int datbuf;
      vector<int>& socklist = sockman.connected_socket_list();
      printf("no. of connected sockets = %d\n", socklist.size());
      for (vector<int>::iterator it = socklist.begin(); it != socklist.end(); ++it) {
        int fd = *it;
        int is = read(fd, &datbuf, 4);
        printf("Data read from sock %d (%d), ret = %d\n", fd, count++, is);
      }
    }
  }
}
