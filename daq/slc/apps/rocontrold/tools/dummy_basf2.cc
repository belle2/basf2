#include <daq/slc/readout/ProcessStatusBuffer.h>

#include <daq/slc/base/Debugger.h>

#include <cstdlib>
#include <unistd.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 4) {
    Belle2::debug("usage : dummy_basf2 <nodeid> <> <nodename>");
  }
  ProcessStatusBuffer m_msg;
  int nodeid = atoi(argv[0]);
  const char* nodename = argv[2];
  Belle2::debug("node = %s, id = %d", nodename, nodeid);
  m_msg.setNode(nodename, nodeid);
  m_msg.open();
  m_msg.reportRunning();
  while (true) {
    sleep(5);
    if (m_msg.getInfo().getState() > 0) break;
  }
  m_msg.reportRunning();
  return 0;
}
