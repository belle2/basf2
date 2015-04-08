#include "daq/slc/apps/mwreaderd/mwreader.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>

#include <cstdlib>
#include <iostream>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("Usage : %s <mynode> <dataname> [<parname>]", argv[0]);
    return 1;
  }
  NSMCommunicator com;
  ConfigFile config("slowcontrol");
  const std::string hostname = config.get("nsm.host");
  const int port = config.getInt("nsm.port");
  com.init(NSMNode(argv[1]), hostname, port);
  NSMData data(argv[2], "mwreader", -1);
  mwreader* reader = (mwreader*)data.open(com);
  const int nitem = reader->nitem;
  std::cout << "nitem  : " << nitem << std::endl;
  std::cout << "date   : " << reader->date << std::endl;
  std::cout << "time   : " << reader->time << std::endl;
  for (int i = 0; i < nitem; i++) {
    mwreader::mwdata& mdata(reader->data[i]);
    std::string vname = StringUtil::form("data[%d].", i);
    std::cout << vname + "unit  : " << mdata.unit << std::endl;
    std::cout << vname + "alarm : " << mdata.alarm << std::endl;
    std::cout << vname + "cond  : " << mdata.cond << std::endl;
    std::cout << vname + "value : " << mdata.value << std::endl;
    std::cout << vname + "chan  : " << mdata.chan << std::endl;
  }
  return 0;
}
