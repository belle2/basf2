#include <daq/slc/xml/XMLParser.h>

#include "daq/slc/apps/hvcontrold/HVControlCallback.h"
#include "daq/slc/apps/hvcontrold/HVControlMaster.h"
#include "daq/slc/apps/hvcontrold/HVCrateInfo.h"
#include "daq/slc/apps/hvcontrold/HVParamProvider.h"

#include "daq/slc/apps/SocketAcceptor.h"

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/PThread.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>

#include <cstdlib>
#include <cstring>
#include <unistd.h>

extern "C" {
#include <readline/readline.h>
#include <readline/history.h>
}

using namespace Belle2;

typedef SocketAcceptor<HVParamProvider, HVControlMaster> HVControlAcceptor;

int main(int argc, char**)
{
  if (argc < 1) {
    Belle2::debug("Usage : ./hvmaster");
    return 1;
  }

  ConfigFile config("slowcontrol", "highvoltage");
  PostgreSQLInterface db(config.get("HV_DATABASE_HOST"),
                         config.get("HV_DATABASE_NAME"),
                         config.get("HV_DATABASE_USER"),
                         config.get("HV_DATABASE_PASS"),
                         config.getInt("HV_DATABASE_PORT"));
  const std::string name = config.get("HV_NSM_NAME");
  NSMNode* node = new NSMNode(name);
  XMLParser parser;
  XMLElement* el = parser.parse(config.get("HV_MONITOR_XML_PATH") + "/" +
                                config.get("HV_MONITOR_XML_ENTRY") + ".xml");
  std::vector<XMLElement*> el_v = el->getElements();
  HVControlMaster* master = new HVControlMaster(db);
  for (size_t i = 0; i < el_v.size(); i++) {
    XMLElement* elc = el_v[i];
    std::string name = elc->getAttribute("name");
    int slot = atoi(elc->getAttribute("slot").c_str());
    int channel = atoi(elc->getAttribute("channel").c_str());
    master->add(name, new HVCrateInfo(i, slot, channel));
  }
  HVControlCallback* callback = new HVControlCallback(node);
  PThread(new NSMNodeDaemon(callback));
  while (!callback->isReady()) sleep(1);
  master->setCommunicator(callback->getCommunicator());
  PThread(new HVControlAcceptor(config.get("HV_GLOBAL_HOST"),
                                config.getInt("HV_GLOBAL_PORT"), master));
  HIST_ENTRY* history = NULL;
  int nhistory = 0;
  char* prompt = (char*)malloc(name.size() + 3);
  strcpy(prompt, (name + "> ").c_str());
  char* line = NULL;
  while (true) {
    line = readline(prompt);
    std::vector<std::string> str_v = Belle2::split(line, ' ', 2);
    add_history(line);
    nhistory++;
    if (nhistory >= 20) {
      history = remove_history(0);
      free(history);
    }
    std::string command = Belle2::toupper(str_v[0]);
    if (command == "QUIT") return 0;
    std::string str = (strlen(line) > command.size()) ? (line + command.size() + 1) : "";
    free(line);
    master->perform(command, Belle2::toupper(str));
  }
  return 0;
}
