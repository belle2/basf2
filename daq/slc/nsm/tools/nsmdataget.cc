#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>

#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 4) {
    LogFile::debug("Usage : %s <mynode> <dataname> <format> [<parname>]", argv[0]);
    return 1;
  }
  NSMCommunicator com;
  ConfigFile config("slowcontrol");
  const std::string hostname = config.get("nsm.host");
  const int port = config.getInt("nsm.port");
  com.init(NSMNode(argv[1]), hostname, port);
  NSMData data(argv[2], argv[3], -1);
  data.open(com);
  DBField::Type type;
  if (argc > 4) {
    const void* p = data.find(argv[4], type);
    if (type == DBField::INT) {
      printf("%s : %d\n", argv[4], *(int*)p);
    } else if (type == DBField::SHORT) {
      printf("%s : %d\n", argv[4], *(short*)p);
    } else if (type == DBField::CHAR) {
      printf("%s : %d\n", argv[4], *(char*)p);
    } else if (type == DBField::FLOAT) {
      printf("%s : %f\n", argv[4], *(float*)p);
    } else if (type == DBField::DOUBLE) {
      printf("%s : %f\n", argv[4], *(double*)p);
    }
  } else {
    data.print();
  }
  return 0;
}
