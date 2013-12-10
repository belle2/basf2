#include "daq/slc/apps/runcontrold/RunStatus.h"
#include "daq/slc/apps/runcontrold/RunConfig.h"
#include "daq/slc/apps/runcontrold/RCMaster.h"

#include <daq/slc/nsm/NSMMessage.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/xml/ObjectLoader.h>
#include <daq/slc/xml/XMLParser.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Command.h>
#include <daq/slc/base/DataObject.h>
#include <daq/slc/base/NSMNode.h>

#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <iostream>

extern "C" {
#include <readline/readline.h>
#include <readline/history.h>
}

using namespace Belle2;

int main(int, char**)
{
  ConfigFile config("slowcontrol", "cdc");
  XMLParser parser;
  XMLElement* el = parser.parse(config.get("RC_XML_PATH") + "/" +
                                config.get("RC_XML_ENTRY") + ".xml");
  const std::string name = el->getAttribute("name");
  HIST_ENTRY* history = NULL;
  int nhistory = 0;
  char* prompt = (char*)malloc(name.size() + 3);
  strcpy(prompt, (name + "> ").c_str());
  char* line = NULL;
  NSMNode* node = new NSMNode("MASTER");
  NSMNode* rc_node = new NSMNode(name);
  NSMCommunicator* nsm_comm = new NSMCommunicator(node);
  nsm_comm->init(config.get("NSM_LOCAL_HOST"),
                 config.getInt("NSM_LOCAL_PORT"));

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
    else {
      NSMMessage msg;
      msg.setNParams(3);
      msg.setParam(0, 0);
      msg.setParam(1, 0);
      msg.setParam(2, 0);
      Command cmd(command.c_str());
      std::string str = (strlen(line) > command.size()) ? (line + command.size() + 1) : "ALL";
      free(line);
      nsm_comm->sendRequest(rc_node, cmd, msg.getNParams(), msg.getParams(), str);
    }
  }
  return 0;
}
