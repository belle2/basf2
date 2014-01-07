#include "daq/slc/apps/envmonitord/EnvMonitorMaster.h"

#include "daq/slc/apps/envmonitord/EnvDBRecorder.h"

#include "daq/slc/apps/PackageSender.h"
#include "daq/slc/apps/SocketAcceptor.h"

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/xml/XMLParser.h>

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/DynamicLoader.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/ConfigFile.h>

#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

typedef SocketAcceptor<PackageSender, EnvMonitorMaster> EnvUIAcceptor;

typedef void* MonitorFunc_t(const char*);

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

  ConfigFile config("slowcontrol");
  DBInterface* db =
    new PostgreSQLInterface(config.get("DATABASE_HOST"),
                            config.get("DATABASE_NAME"),
                            config.get("DATABASE_USER"),
                            config.get("DATABASE_PASS"),
                            config.getInt("DATABASE_PORT"));
  EnvDBRecorder::setDB(db);

  std::vector<DynamicLoader*> dl_v;
  XMLParser parser;
  XMLElement* el = parser.parse(config.get("ENV_XML_PATH") + "/" +
                                config.get("ENV_XML_ENTRY") + ".xml");
  std::vector<XMLElement*> el_v = el->getElements();
  EnvMonitorMaster* master = new EnvMonitorMaster(comm);
  for (size_t i = 0; i < el_v.size(); i++) {
    XMLElement* elc = el_v[i];
    if (elc->getTag() != "monitor") continue;
    std::string monitor_name = elc->getAttribute("name");
    std::string monitor_class = elc->getAttribute("class");
    std::string lib_name = elc->getAttribute("lib");
    if (lib_name.at(0) != '/') lib_name = "lib" + lib_name + ".so";
    DynamicLoader* dl = new DynamicLoader();
    dl->open(lib_name);
    MonitorFunc_t* createMonitor =
      (MonitorFunc_t*)dl->load(Belle2::form("create%s",
                                            monitor_class.c_str()));
    EnvMonitorPackage* monitor = (EnvMonitorPackage*)createMonitor(monitor_name.c_str());
    dl_v.push_back(dl);
    std::vector<XMLElement*> elc_v = elc->getElements();
    for (size_t i = 0; i < elc_v.size(); i++) {
      XMLElement* elcc = elc_v[i];
      if (elcc->getTag() != "data") continue;
      std::string dataname = elcc->getAttribute("name");
      std::string format = elcc->getAttribute("format");
      int revision = atoi(elcc->getAttribute("revision").c_str());
      monitor->addData(new NSMData(dataname, format, revision));
    }
    master->add(monitor);
  }
  PThread(new EnvUIAcceptor(config.get("ENV_GUI_HOST"),
                            config.getInt("ENV_GUI_PORT"), master));
  master->run();
  return 0;
}
