#include <cstdio>

#include <daq/slc/system/UDPSocket.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  UDPSocket socket(12345, "192.168.10.255", true);
  socket.write("HELLO", 5);
  socket.close();
  return 0;
}
