#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/runcontrol/RCCommand.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 4) {
    LogFile::debug("Usage : %s <myname> <nodename> <command> [<message>]",
                   argv[0]);
    return 1;
  }
  ConfigFile config("slowcontrol");
  NSMCommunicator com;
  com.init(NSMNode(StringUtil::toupper(argv[1])),
           config.get("nsm.host"), config.getInt("nsm.port"));
  NSMNode node(StringUtil::toupper(argv[2]));
  std::string request = StringUtil::toupper(StringUtil::replace(argv[3], ":", "_"));
  if (!StringUtil::find(request, "RC_")) {
    request = "RC_" + request;
  }
  RCCommand cmd(request);
  if (argc > 4) {
    if (cmd == RCCommand::START) {
      int pars[256];
      for (int i = 4; i < argc; i++) {
        pars[i - 4] = atoi(argv[i]);
      }
      NSMCommunicator::send(NSMMessage(node, cmd, argc - 4, pars));
    } else {
      NSMCommunicator::send(NSMMessage(node, cmd, argv[4]));
    }
  } else {
    NSMCommunicator::send(NSMMessage(node, cmd));
  }
  return 0;
}
