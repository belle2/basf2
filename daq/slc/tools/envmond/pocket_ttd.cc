#include <nsm/NSMNodeDaemon.h>
#include <nsm/NSMData.h>
#include <nsm/NSMCommunicator.h>

#include "pocket_ttd.h"

#include <database/MySQLInterface.h>

#include <base/Debugger.h>
#include <base/StringUtil.h>

#include <iostream>
#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage : ./recvd <name>");
    return 1;
  }
  const char* name = argv[1];

  NSMNode* node = new NSMNode(name);
  NSMCommunicator* comm = new NSMCommunicator(node);
  comm->init();
  NSMData* data = new NSMData("TTD_STATUS", "pocket_ttd", 5);
  data->allocate();
  std::cout << data->toSQLConfig() << std::endl;
  std::cout << data->toSQLValues() << std::endl;
  pocket_ttd* status = (pocket_ttd*)data->get();
  while (true) {
    status->evt_number++;
    status->run_number += 10;
    status->exp_number += 20;
    status->staa[0] += 2;
    status->stab[5] += 2;
    std::cout << data->toSQLValues() << std::endl;
    sleep(5);
  }

  return 0;
}
