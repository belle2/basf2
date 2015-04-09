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
  if (argc > 4) {
    DBField::Type type;
    int length;
    for (int i = 4; i < argc; i++) {
      const void* p = data.find(argv[i], type, length);
      switch (type) {
        case DBField::LONG:
          printf("%s : %lld\n", argv[i], *(long long*)p);
          break;
        case DBField::INT:
          printf("%s : %d\n", argv[i], *(int*)p);
          break;
        case DBField::SHORT:
          printf("%s : %d\n", argv[i], *(short*)p);
          break;
        case DBField::CHAR:
          if (length > 0) {
            printf("%s : %s\n", argv[i], (const char*)p);
          } else {
            printf("%s : %d\n", argv[i], *(char*)p);
          }
          break;
        case DBField::FLOAT:
          printf("%s : %f\n", argv[i], *(float*)p);
          break;
        case DBField::DOUBLE:
          printf("%s : %f\n", argv[i], *(double*)p);
          break;
        default:
          break;
      }
    }
  } else {
    data.print();
  }
  return 0;
}
