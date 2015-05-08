#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/nsm/nsm_read_argv.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <unistd.h>


int help(const char** argv)
{
  printf("usage : %s <nodename> <command> [<message>] "
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
  NSMCommunicator com;
  com.init(NSMNode(StringUtil::toupper(username)),
           config.get(name + ".host"), config.getInt(name + ".port"));
  NSMNode node(StringUtil::toupper(argv_in[1]));
  std::string request = StringUtil::toupper(StringUtil::replace(argv_in[2], ":", "_"));
  NSMCommand cmd(1000, request.c_str());
  if (argc_in > 3) {
    NSMCommunicator::send(NSMMessage(node, cmd, argv_in[3]));
  } else {
    NSMCommunicator::send(NSMMessage(node, cmd));
  }
  return 0;
}
