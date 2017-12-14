#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <daq/slc/system/Executor.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>

using namespace Belle2;

int main(int argc, char** argv)
{
  ConfigFile config("slowcontrol");
  std::string name = "nsm";
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-g") == 0) {
      name = "nsm.global";
    } else if (strcmp(argv[i], "-h") == 0) {
      printf("usage  : %s [<conf1> [<conf2>]...] [-g]\n", argv[0]);
      printf("options: -g : use nsm.global (default:nsm)\n");
      return 0;
    } else {
      config.read(argv[i]);
    }
  }
  int port = config.getInt(name + ".port");
  std::string cmd = StringUtil::form("nsminfo2 -s %d -p %d", port, port);
  //printf("%s\n", cmd.c_str());
  system(cmd.c_str());
  return 0;
}

