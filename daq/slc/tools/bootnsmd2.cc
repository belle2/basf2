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
    config.read(argv[i]);
    if (strcmp(argv[i], "global") == 0) {
      name = "nsm.global";
    }
  }
  int port = config.getInt(name + ".port");
  std::string host = config.get(name + ".host");
  std::string logdir = config.get("logfile.dir");
  std::string cmd = StringUtil::form("nsmd2 -b -s %d -p %d -h %s -l %s/nsm2 -o > /dev/null&",
                                     port, port, host.c_str(), logdir.c_str());
  //std::string cmd = StringUtil::form("nsmd2 -s %d -p %d -h %s -l %s/nsm2",
  //                                   port, port, host.c_str(), logdir.c_str());
  printf("booting nsmd2\n");
  printf("%s\n", cmd.c_str());
  system(cmd.c_str());
  return 0;
}

