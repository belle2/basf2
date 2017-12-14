#include <daq/slc/readout/IOInfo.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <cstdlib>
#include <cstdio>
#include <iostream>

int main(int argc, char** argv)
{
  if (argc < 2) {
    printf("usage : %s hostname port\n", argv[0]);
  }
  char* hostname = argv[1];
  int port = atoi(argv[2]);
  struct hostent* host = NULL;
  host = gethostbyname(hostname);
  if (host == NULL) {
    unsigned long ip_address = inet_addr(hostname);
    if ((signed long) ip_address < 0) {
      printf("Wrong ip address\n");
      return 1;
    } else {
      host = gethostbyaddr((char*)&ip_address, sizeof(ip_address), AF_INET);
    }
  }
  unsigned int addr = (*(unsigned long*) host->h_addr_list[0]);
  Belle2::IOInfo ioinfo;
  ioinfo.setLocalAddress(addr);
  ioinfo.setLocalPort(port);
  Belle2::IOInfo::checkTCP(ioinfo);
  std::cout << "local ip = " << ioinfo.getLocalIP() << std::endl
            << "local address = " << ioinfo.getLocalAddress() << std::endl
            << "local port = " << ioinfo.getLocalPort() << std::endl
            << "remote ip = " << ioinfo.getRemoteIP() << std::endl
            << "remote address = " << ioinfo.getRemoteAddress() << std::endl
            << "remote port = " << ioinfo.getRemotePort() << std::endl
            << "state = " << ioinfo.getState() << std::endl
            << "txqueue = " << ioinfo.getTXQueue() << std::endl
            << "rxqueue = " << ioinfo.getRXQueue() << std::endl;
  return 0;
}
