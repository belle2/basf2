#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/nsm/nsm_read_argv.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>

#include <cstdlib>

int help(const char** argv)
{
  printf("usage : %s <dataname> <format> [<varname>...] "
         "[-n myname] [-c conf] [-g]\n", argv[0]);
  printf("options: -c : set conf file \"conf\" (default:slowcontrol)\n");
  printf("options: -n : set nsm user name (default:env of USER)\n");
  printf("options: -g : use nsm.global (default:nsm)\n");
  return 0;
}

using namespace Belle2;

int main(int argc, const char** argv)
{
  ConfigFile config("slowcontrol");
  std::string name, username;
  char** argv_in = new char* [argc];
  int argc_in = nsm_read_argv(argc, argv, help, argv_in, config, name, username, 3);
  const std::string hostname = config.get(name + ".host");
  const int port = config.getInt(name + ".port");
  NSMCommunicator com;
  com.init(NSMNode(username), hostname, port);
  NSMData data(argv_in[1], argv_in[2], -1);
  data.open(com);
  if (argc_in > 3) {
    DBField::Type type;
    int length;
    for (int i = 3; i < argc; i++) {
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
