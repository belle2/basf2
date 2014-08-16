#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <daq/slc/system/UDPSocket.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  UDPSocket socket;
  socket.bind(atoi(argv[2]), argv[1]);
  char buf[2048];
  while (true) {
    memset(buf, 0, sizeof(buf));
    socket.read(buf, sizeof(buf));
    printf("%s:%d\n",
           socket.getRemoteHostName().c_str(),
           socket.getRemotePort());
    printf("%s\n", buf);
    UDPSocket socket_r(atoi(argv[3]),
                       socket.getRemoteHostName());
    socket_r.write("HELLO", 5);
    socket_r.close();
  }
  socket.close();
  return 0;
}
