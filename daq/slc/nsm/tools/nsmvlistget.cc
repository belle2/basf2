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
      NSMNode node(m_argv[2]);
      try {
        NSMCommunicator::connected(node.getName());
      } catch (const NSMNotConnectedException& e) {
        printf("node %s is not online\n", m_argv[2]);
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

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("Usage : %s <myname> <nodename>", argv[0]);
    return 1;
  }
  ConfigFile config("slowcontrol");
  const std::string hostname = config.get("nsm.host");
  const int port = config.getInt("nsm.port");
  NSMVLISTGETCallback* callback = new NSMVLISTGETCallback(NSMNode(argv[1]), argc, argv);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback, hostname, port);
  daemon->run();
  return 0;
}
