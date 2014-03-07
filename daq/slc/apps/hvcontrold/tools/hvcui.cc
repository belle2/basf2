#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>

#include "daq/slc/apps/hvcontrold/HVCommand.h"
#include "daq/slc/apps/hvcontrold/HVState.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>

extern "C" {
#include <readline/readline.h>
#include <readline/history.h>
}

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage : ./hvcui nodename");
    return 1;
  }
  ConfigFile config("slowcontrol", "hv");
  std::string nodename = argv[1];
  NSMNode* node = new NSMNode(nodename);
  NSMNode* hvnode = new NSMNode(config.get("hv.nsm.name"));
  NSMCommunicator* comm = new NSMCommunicator(node);
  comm->init(config.get("nsm.local.host"), config.getInt("nsm.local.port"));
  HIST_ENTRY* history = NULL;
  int nhistory = 0;
  char* prompt = (char*)malloc(nodename.size() + 3);
  strcpy(prompt, (nodename + "> ").c_str());
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
    HVCommand cmd = command;
    std::cout << cmd.getLabel() << std::endl;
    try {
      if (cmd == HVCommand::LOAD) {
        std::vector<std::string> sarg_v = Belle2::split(str, ' ');
        if (sarg_v.size() >= 2) {
          int npar = 2;
          int pars[2] = {1, atoi(sarg_v[1].c_str())};
          comm->sendRequest(hvnode, cmd, npar, pars, sarg_v[0]);
        } else {
          comm->sendRequest(hvnode, cmd);
        }
      } else {
        comm->sendRequest(hvnode, cmd);
      }
    } catch (const NSMHandlerException& e) {

    }
  }

  return 0;
}

