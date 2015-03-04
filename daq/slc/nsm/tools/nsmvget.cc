#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

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
      : NSMCallback(5), m_argc(argc), m_argv(argv) {
      setNode(node);
    }
    virtual ~NSMVGETCallback() throw() {}

  public:
    virtual void init(NSMCommunicator&) throw() {
      try {
        NSMNode node(m_argv[2]);
        NSMCommunicator::connected(node.getName());
        for (int i = 3; i < m_argc; i++) {
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
        printf("node %s is not online\n", m_argv[2]);
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

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("Usage : %s <myname> <nodename> <varname>", argv[0]);
    return 1;
  }
  ConfigFile config("slowcontrol");
  const std::string hostname = config.get("nsm.host");
  const int port = config.getInt("nsm.port");
  NSMVGETCallback* callback = new NSMVGETCallback(NSMNode(argv[1]), argc, argv);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback, hostname, port);
  daemon->run();
  return 0;
}
