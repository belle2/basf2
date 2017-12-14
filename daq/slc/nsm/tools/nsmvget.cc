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

  class NSMVGETCallback : public NSMCallback {

  public:
    NSMVGETCallback(const NSMNode& node, int argc, char** argv)
      : NSMCallback(5), m_argc(argc), m_argv(argv)
    {
      setNode(node);
    }
    virtual ~NSMVGETCallback() throw() {}

  public:
    virtual void init(NSMCommunicator&) throw()
    {
      try {
        NSMNode node(m_argv[1]);
        NSMCommunicator::connected(node.getName());
        for (int i = 2; i < m_argc; i++) {
          NSMVar var(m_argv[i]);
          get(node, var);
          switch (var.getType()) {
            case NSMVar::INT:
              printf("%s : %d\n", var.getName().c_str(), var.getInt());
              break;
            case NSMVar::FLOAT:
              printf("%s : %f\n", var.getName().c_str(), var.getFloat());
              break;
            case NSMVar::TEXT:
              printf("%s : %s\n", var.getName().c_str(), var.getText());
              break;
            default:
              break;
          }
        }
      } catch (const NSMNotConnectedException& e) {
        printf("node %s is not online\n", m_argv[1]);
      } catch (const NSMHandlerException& e) {
        printf("NSM error %s\n", e.what());
      } catch (const IOException& e) {
        printf("timeout\n");
      }
      exit(1);
    }

  private:
    int m_argc;
    char** m_argv;
  };

}

int help(const char** argv)
{
  printf("usage  : %s <nodename> <varname> [-n myname] [-c conf] [-g]\n", argv[0]);
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
  int port = config.getInt(name + ".port");
  std::string host = config.get(name + ".host");
  NSMVGETCallback* callback = new NSMVGETCallback(NSMNode(username), argc_in, argv_in);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback, host, port);
  daemon->run();
  return 0;
}
