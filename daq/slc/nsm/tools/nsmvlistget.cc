#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/nsm/nsm_read_argv.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <unistd.h>
#include <cstdlib>
#include <cstdio>

namespace Belle2 {

  class NSMVLISTGETCallback : public NSMCallback {

  public:
    NSMVLISTGETCallback(const NSMNode& node, int argc, char** argv)
      : NSMCallback(5), m_argc(argc), m_argv(argv)
    {
      setNode(node);
    }
    virtual ~NSMVLISTGETCallback() throw() {}

  public:
    virtual void init(NSMCommunicator&) throw()
    {
      NSMNode node(m_argv[1]);
      try {
        NSMCommunicator::connected(node.getName());
      } catch (const NSMNotConnectedException& e) {
        printf("node %s is not online\n", m_argv[1]);
        exit(1);
      }
      NSMCommunicator::send(NSMMessage(node, NSMCommand::VLISTGET));
    }
    virtual void timeout(NSMCommunicator&) throw()
    {
      printf("timeout\n");
      exit(1);
    }
    virtual void vlistset(NSMCommunicator& com) throw()
    {
      NSMCallback::vlistset(com);
      exit(1);
    }

  private:
    int m_argc;
    char** m_argv;
  };

}

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
  int argc_in = nsm_read_argv(argc, argv, help, argv_in, config, name, username, 2);
  const std::string hostname = config.get(name + ".host");
  const int port = config.getInt(name + ".port");
  NSMVLISTGETCallback* callback = new NSMVLISTGETCallback(NSMNode(username), argc_in, argv_in);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback, hostname, port);
  daemon->run();
  return 0;
}
