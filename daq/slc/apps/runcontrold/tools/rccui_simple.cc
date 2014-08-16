#include "daq/slc/runcontrol/RCCommand.h"
#include "daq/slc/runcontrol/RCState.h"

#include "daq/slc/apps/runcontrold/rc_status.h"

#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/database/ConfigObjectTable.h>
#include <daq/slc/database/ConfigInfoTable.h>
#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

extern "C" {
#include <readline/readline.h>
#include <readline/history.h>
}

using namespace Belle2;

struct runsetting {
  std::string configname;
  std::string operators;
  std::string comment;
  bool stored;
} setting = {"", "", "", false};

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("Usage : ./rccui nodename rcnode");
    return 1;
  }
  ConfigFile config("slowcontrol", "rc");
  std::string nodename = argv[1];
  NSMNode node(nodename);
  std::vector<NSMNode> rcnode_v;
  for (int i = 1; i < argc; i++) {
    rcnode_v.push_back(NSMNode(argv[i]));
  }
  NSMCommunicator* comm = new NSMCommunicator();
  comm->init(node, config.get("nsm.local.host"),
             config.getInt("nsm.local.port"));
  HIST_ENTRY* history = NULL;
  int nhistory = 0;
  char* prompt = (char*)malloc(nodename.size() + 3);
  strcpy(prompt, (nodename + "> ").c_str());
  char* line = NULL;
  while (true) {
    line = readline(prompt);
    std::vector<std::string> str_v = StringUtil::split(line, ' ', 2);
    add_history(line);
    nhistory++;
    if (nhistory >= 20) {
      history = remove_history(0);
      free(history);
    }
    fputs("\033[2J\033[0;0H", stdout);
    rewind(stdout);
    ftruncate(1, 0);
    std::string command = StringUtil::toupper(str_v[0]);
    if (command == "QUIT") return 0;
    std::string str = (strlen(line) > command.size()) ? (line + command.size() + 1) : "";
    free(line);
    RCCommand cmd = command;
    if (cmd == Enum::UNKNOWN) {
      std::cout << "Unknown command : " << command << std::endl;
      continue;
    }
    try {
      for (size_t i = 0; i < rcnode_v.size(); i++) {
        comm->sendRequest(NSMMessage(rcnode_v[i], cmd));
      }
    } catch (const NSMHandlerException& e) {
    }
  }

  return 0;
}

