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

  class NSMVSETCallback : public NSMCallback {

  public:
    NSMVSETCallback(const NSMNode& node, int argc, char** argv)
      : NSMCallback(5), m_argc(argc), m_argv(argv) {
      setNode(node);
    }
    virtual ~NSMVSETCallback() throw() {}

  public:
    virtual void init(NSMCommunicator&) throw() {
      std::string type = m_argv[4];
      try {
        bool result = true;
        const std::string name = m_argv[3];
        NSMNode node(m_argv[2]);
        if (type == "int") {
          result = set(node, NSMVar(name, (int)atoi(m_argv[5])));
        } else if (type == "float") {
          result = set(node, NSMVar(name, (float)atof(m_argv[5])));
        } else if (type == "text") {
          result = set(node, NSMVar(name, m_argv[5]));
        }
        printf("%s %s\n", name.c_str(), (result ? "SUCCESS" : "FAILED"));
      } catch (const NSMHandlerException& e) {
        printf("nsm2 error %s\n", e.what());
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
  if (argc < 6) {
    LogFile::debug("Usage : %s <myname> <nodename> <varname> <type=int/float/text> <value>", argv[0]);
    return 1;
  }
  ConfigFile config("slowcontrol");
  const std::string hostname = config.get("nsm.host");
  const int port = config.getInt("nsm.port");
  NSMVSETCallback* callback = new NSMVSETCallback(NSMNode(argv[1]), argc, argv);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback, hostname, port);
  daemon->run();
  return 0;
}
