#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/apps/storagerd/storage_status.h>

#include <daq/slc/system/LogFile.h>

#include <unistd.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("Usage : %s <dataname> <format> [<parname>]", argv[0]);
    return 1;
  }
  NSMCommunicator com;
  com.init(NSMNode("READER"), "b2slow2.kek.jp", 9122);
  NSMData data(argv[1], argv[2], -1);
  data.open(com);
  DBField::Type type;
  if (argc > 3) {
    const void* p = data.find(argv[3], type);
    if (type == DBField::INT) {
      printf("%s : %d\n", argv[3], *(int*)p);
    } else if (type == DBField::SHORT) {
      printf("%s : %d\n", argv[3], *(short*)p);
    } else if (type == DBField::CHAR) {
      printf("%s : %d\n", argv[3], *(char*)p);
    } else if (type == DBField::FLOAT) {
      printf("%s : %f\n", argv[3], *(float*)p);
    } else if (type == DBField::DOUBLE) {
      printf("%s : %f\n", argv[3], *(double*)p);
    }
  } else {
    data.print();
  }
  return 0;
}
