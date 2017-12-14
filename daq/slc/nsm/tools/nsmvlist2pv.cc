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
#include <iostream>
#include <sstream>

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
      std::string data = com.getMessage().getData();
      StringList s = StringUtil::split(data, '\n');
      for (size_t i = 0; i < s.size(); i++) {
        StringList ss = StringUtil::split(s[i], ':');
        if (ss.size() > 4) {
          std::string name = StringUtil::replace(ss[0], ".", ":");
          //int id = atoi(ss[4].c_str());
          std::string type = ss[1];
          bool useGet = (ss[2] == "1");
          bool useSet = (ss[3] == "1");
          if (useSet) {
            std::stringstream s1, s2;
            if (type == "text") {
              s1 << "stringout";
              s2 << "  field(VAL, \"\")" << std::endl
                 << "  field(DTYP, \"nsm2_vset_stringout\")";
            } else if (type == "int") {
              s1 << "longout";
              s2 << "  field(VAL, \"0\")" << std::endl
                 << "  field(DTYP, \"nsm2_vset_longout\")";
            } else if (type == "float") {
              s1 << "ao";
              s2 << "  field(VAL, \"0\")" << std::endl
                 << "  field(DTYP, \"nsm2_vset_ao\")";
            }
            std::cout << "record(" << s1.str()
                      << ", \"B2_nsm:set:$(nodename):" << name << "\")" << std::endl
                      << "{" << std::endl
                      //<< "  field(DESC, \"set value to " << name << "\")" << std::endl
                      << "  field(DESC, \"\")" << std::endl
                      << "  field(SCAN, \"Passive\")" << std::endl
                      << s2.str() << std::endl
                      << "}" << std::endl
                      << "" << std::endl;
          }
          if (useGet) {
            std::stringstream s1, s2;
            if (type == "text") {
              s1 << "stringin";
              s2 << "  field(VAL, \"\")" << std::endl
                 << "  field(DTYP, \"nsm2_vget_stringin\")";
            } else if (type == "int") {
              s1 << "longin";
              s2 << "  field(VAL, \"0\")" << std::endl
                 << "  field(DTYP, \"nsm2_vget_longin\")";
            } else if (type == "float") {
              s1 << "ai";
              s2 << "  field(VAL, \"0\")" << std::endl
                 << "  field(DTYP, \"nsm2_vget_ai\")";
            }
            std::cout << "record(" << s1.str()
                      << ", \"B2_nsm:get:$(nodename):" << name << "\")" << std::endl
                      << "{" << std::endl
                      //<< "  field(DESC, \"get value from " << name << "\")" << std::endl
                      << "  field(DESC, \"\")" << std::endl
                      << "  field(SCAN, \"I/O Intr\")" << std::endl
                      << s2.str() << std::endl
                      << "}" << std::endl
                      << "" << std::endl;
          }
        }
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
