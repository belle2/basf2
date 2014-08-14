#include <cstdio>
#include <cstring>

#include <daq/slc/system/UDPSocket.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  UDPSocket socket;
  socket.bind(12345, argv[1]);
  char buf[2048];
  while (true) {
    memset(buf, 0, sizeof(buf));
    socket.read(buf, sizeof(buf));
    printf("%s\n", buf);
  }
  socket.close();
  return 0;
}
