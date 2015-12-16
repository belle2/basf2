#include "daq/slc/apps/socketmonitord/SocketmonitorCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    std::string host = config.get("nsm.host");
    int port = config.getInt("nsm.port");
    std::string name = config.get("nsm.nodename");
    int timeout = config.getInt("timeout");
    if (timeout <= 0) timeout = 5;
    SocketmonitorCallback* callback = new SocketmonitorCallback(name);
    std::vector<std::string> s = StringUtil::split(config.get("sockets"), ',');
    for (size_t i = 0; i < s.size(); i++) {
      std::vector<std::string> ss = StringUtil::split(s[i], ':', 3);
      callback->addSocket(ss[0], atoi(ss[1].c_str()),
                          StringUtil::tolower(ss[2]) == "local");
    }
    NSMNodeDaemon(callback, host, port).run();
  }
  return 0;
}
