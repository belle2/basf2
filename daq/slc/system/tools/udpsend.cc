#include <cstdio>

#include <daq/slc/system/UDPSocket.h>

using namespace Belle2;

int main()
{
  //UDPSocket socket(12345, "0.0.0.0", true);
  UDPSocket socket(12345, "255.255.255.255", true);
  socket.write("HELLO", 5);
  socket.close();
  return 0;
}
