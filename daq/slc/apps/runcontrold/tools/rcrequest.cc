#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/runcontrol/RCCommand.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

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
           config.get("nsm.local.host"),
           config.getInt("nsm.local.port"));
  NSMNode node(StringUtil::toupper(argv[2]));
  std::string request = StringUtil::toupper(argv[3]);
  if (!StringUtil::find(request, "RC_")) {
    request = "RC_" + request;
  }
  RCCommand cmd(request);
  if (argc > 4) {
    NSMCommunicator::send(NSMMessage(node, cmd, argv[4]));
  } else {
    NSMCommunicator::send(NSMMessage(node, cmd));
  }
  return 0;
}
