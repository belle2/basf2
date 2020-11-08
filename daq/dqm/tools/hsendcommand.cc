#include <daq/dataflow/EvtSocket.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

#include "TText.h"

#include <stdio.h>
#include <string>

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 4) {
    printf("Usage : hsendcommand cmd host port\n");
    exit(-1);
  }
  string cmd = string(argv[1]);
  string host = string(argv[2]);
  int port = atoi(argv[3]);

  EvtSocketSend* sock = new EvtSocketSend(host.c_str(), port);

  MsgHandler hdl(0);
  int numobjs = 0;

  TText command(0, 0, cmd.c_str());
  hdl.add(&command, cmd.c_str());
  numobjs++;

  EvtMessage* msg = hdl.encode_msg(MSG_EVENT);
  (msg->header())->reserved[0] = 0;
  (msg->header())->reserved[1] = numobjs;

  sock->send(msg);
  delete(msg);
  delete(sock);

  return 0;
}
