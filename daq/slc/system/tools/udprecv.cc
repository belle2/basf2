#include <cstdio>

#include <daq/slc/system/UDPSocket.h>

using namespace Belle2;

int main()
{
  UDPSocket socket;
  socket.bind(12345);
  char buf[2048];
  while (true) {
    socket.read(buf, sizeof(buf));
    printf("%s\n", buf);
  }
  socket.close();
  return 0;
}
