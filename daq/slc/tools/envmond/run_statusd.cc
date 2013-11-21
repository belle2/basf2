#include <nsm/NSMNodeDaemon.h>
#include <nsm/NSMData.h>
#include <nsm/NSMCommunicator.h>

#include "run_status.h"

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
  NSMData* data = new NSMData("RUN_STATUS", "run_status", 1);
  data->allocate();
  std::cout << data->toSQLConfig() << std::endl;
  std::cout << data->toSQLValues() << std::endl;
  run_status* status = (run_status*)data->get();
  while (true) {
    status->serial++;
    status->run_no += 100;
    status->exp_no += 200;
    std::cout << data->toSQLValues() << std::endl;
    sleep(5);
  }

  return 0;
}
