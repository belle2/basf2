#include "daq/slc/apps/dqmviewd/DQMViewMaster.h"
#include "daq/slc/apps/dqmviewd/SimpleDQMPackage.h"

#include <daq/slc/apps/PackageSender.h>
#include <daq/slc/apps/SocketAcceptor.h>

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/xml/XMLParser.h>

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/DynamicLoader.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/ConfigFile.h>

#include <cstdlib>

using namespace Belle2;

typedef SocketAcceptor<PackageSender, DQMViewMaster> DQMUIAcceptor;

typedef void* MonitorFunc_t(const char*, const char*);

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage: ./dqmserver <name>");
    return 1;
  }
  const char* name = argv[1];
  ConfigFile config("slowcontrol");
  std::vector<DynamicLoader*> dl_v;
  XMLParser parser;
  XMLElement* el = parser.parse(config.get("DQM_XML_PATH") + "/DQM.xml");
  std::vector<XMLElement*> el_v = el->getElements();
  DQMViewMaster* master = new DQMViewMaster();
  std::string data_path = config.get("DQM_DATA_PATH");
  master->setDirectory(data_path);
  ::system(Belle2::form("mkdir -p %s", data_path.c_str()).c_str());
  for (size_t i = 0; i < el_v.size(); i++) {
    XMLElement* elc = el_v[i];
    if (elc->getTag() != "monitor") continue;
    std::string file_name = elc->getAttribute("file");
    std::string monitor_name = elc->getAttribute("name");
    std::string monitor_class = elc->getAttribute("class");
    std::string lib_name = elc->getAttribute("lib");
    if (monitor_class.size() > 0) {
      if (lib_name.size() > 0 && lib_name.at(0) != '/') lib_name = "lib" + lib_name + ".so";
      DynamicLoader* dl = new DynamicLoader();
      dl->open(lib_name);
      MonitorFunc_t* createMonitor =
        (MonitorFunc_t*)dl->load(Belle2::form("create%s",
                                              monitor_class.c_str()));
      DQMPackage* monitor = (DQMPackage*)createMonitor(monitor_name.c_str(),
                                                       file_name.c_str());
      dl_v.push_back(dl);
      master->add(monitor);
    } else {
      master->add(new SimpleDQMPackage(monitor_name, file_name));
    }
  }
  Belle2::PThread(new DQMUIAcceptor(config.get("DQM_GUI_HOST"),
                                    config.getInt("DQM_GUI_PORT"), master));
  master->run();
  return 0;
}

