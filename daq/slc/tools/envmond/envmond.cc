#include <xml/XMLParser.h>

#include "EnvMonitorMaster.h"
#include "DBManager.h"
#include "SocketAcceptor.h"

#include <database/MySQLInterface.h>

#include <nsm/NSMData.h>
#include <nsm/NSMCommunicator.h>

#include <system/PThread.h>

#include <base/StringUtil.h>
#include <base/Debugger.h>
#include <base/ConfigFile.h>

#include <unistd.h>
#include <cstdlib>

#include <dlfcn.h>

using namespace Belle2;

typedef void* MonitorFunc_t();

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage : ./nsmmond <name>");
    return 1;
  }
  const char* name = argv[1];
  NSMNode* node = new NSMNode(name);
  NSMCommunicator* comm = new NSMCommunicator(node);
  comm->init();

  ConfigFile config("slc_config");
  DBInterface* db = new MySQLInterface();
  db->init();
  db->connect(config.get("HV_DATABASE_HOST"), config.get("HV_DATABASE_NAME"),
              config.get("HV_DATABASE_USER"), config.get("HV_DATABASE_PASS"),
              config.getInt("HV_DATABASE_PORT"));
  DBManager::setDB(db);

  XMLParser parser;
  XMLElement* el = parser.parse(config.get("HV_MONITOR_XML_PATH") + "/" +
                                config.get("HV_MONITOR_XML_ENTRY") + ".xml");
  std::vector<XMLElement*> el_v = el->getElements();
  EnvMonitorMaster* master = new EnvMonitorMaster(comm);
  for (size_t i = 0; i < el_v.size(); i++) {
    XMLElement* elc = el_v[i];
    std::string nodename = elc->getAttribute("name");
    std::vector<XMLElement*> elc_v = elc->getElements();
    for (size_t i = 0; i < elc_v.size(); i++) {
      XMLElement* elcc = elc_v[i];
      std::string dataname = elcc->getAttribute("name");
      std::string format = elcc->getAttribute("format");
      std::string monitor_class = elcc->getAttribute("monitor_class");
      int revision = atoi(elcc->getAttribute("revision").c_str());
      AbstractMonitor* monitor = NULL;
      if (monitor_class.size() > 0) {
        void* handle = dlopen(Belle2::form("%s/libdaq_slc_dqm_%s.so",
                                           config.get("DQM_LIB_PATH").c_str(),
                                           monitor_class.c_str()).c_str(),
                              RTLD_NOW | RTLD_GLOBAL);
        if (!handle) {
          Belle2::debug("%s", dlerror());
          return 1;
        }
        char* error = NULL;
        MonitorFunc_t* createMonitor =
          (MonitorFunc_t*)dlsym(handle, Belle2::form("create%sMonitor",
                                                     monitor_class.c_str()).c_str());
        if ((error = dlerror()) != NULL) {
          Belle2::debug("%s", error);
          return 1;
        }
        monitor = (AbstractMonitor*)createMonitor();
      }
      master->add(new NSMData(dataname, format, revision), monitor);
    }
  }
  PThread(new SocketAcceptor(config.get("DQM_GUI_HOST"),
                             config.getInt("DQM_GUI_PORT"), master));
  master->run();
  return 0;
}
