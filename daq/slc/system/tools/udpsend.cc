#include <cstdio>
#include <unistd.h>

#include <daq/slc/system/UDPSocket.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("usage: %s port hostname", argv[0]);
    return 1;
  }
  UDPSocket socket(atoi(argv[2]), argv[1], true);
  socket.write("HELLO", 5);
  socket.close();
  return 0;
}
