#include "daq/slc/apps/runcontrold/RunStatus.h"
#include "daq/slc/apps/runcontrold/RunConfig.h"
#include "daq/slc/apps/runcontrold/RCMaster.h"

#include <daq/slc/nsm/NSMMessage.h>

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
#include <daq/slc/base/Date.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <sstream>

extern "C" {
#include <readline/readline.h>
#include <readline/history.h>
}

namespace Belle2 {

  class SocketListener {
  public:
    SocketListener(const TCPSocket& socket, RCMaster* master)
      : _socket(socket), _master(master) {}

  public:
    void run() {
      TCPSocketWriter writer(_socket);
      TCPSocketReader reader(_socket);
      while (true) {
        int number = reader.readInt();
        _master->getNode()->setConnection(Connection::ONLINE);
        Command cmd = number;
        if (cmd == Command::STATE) {
          std::string name = reader.readString();
          //std::cout << name << std::endl;
          NSMNode* node = NULL;
          if (name == _master->getNode()->getName()) {
            node = _master->getNode();
          } else {
            node = _master->getNodeByName(name);
          }
          int state_number = reader.readInt();
          int connection_number = reader.readInt();
          if (node != NULL) {
            node->setState(State(state_number));
            node->setConnection(Connection(connection_number));
            _master->getStatus()->update();
          }
        } else if (cmd == Command::DATA) {
          std::string name = reader.readString();
          // std::cout << name << std::endl;
          if (name == _master->getNode()->getName()) {
            _master->getData()->readObject(reader);
          } else {
            if (!_master->getData()->getObject(name)) {
              _master->getData()->getObject(name)->readObject(reader);
            }
          }
        } else if (cmd == Command::UNKNOWN) {
          _master->getNode()->setConnection(Connection::OFFLINE);
          return;
        }
      }
    }

  private:
    TCPSocket _socket;
    RCMaster* _master;

  };

}

using namespace Belle2;

std::string getStateCode(const State& state)
{
  std::string state_code = "\x1b[49m\x1b[39m";
  if (state == State::RUNNING_S) {
    state_code = "\x1b[42m\x1b[37m";
  } else if (state.isStable()) {
    state_code = "\x1b[44m\x1b[37m";
  } else if (state.isTransaction()) {
    state_code = "\x1b[43m\x1b[37m";
  } else if (state.isRecovering()) {
    state_code = "\x1b[45m\x1b[37m";
  } else if (state.isError()) {
    state_code = "\x1b[41m\x1b[37m";
  }
  return state_code;
}

std::string getConnectionCode(const Connection& connection)
{
  std::string connection_code = "\x1b[49m\x1b[39m";
  if (connection == Connection::ONLINE) {
    connection_code = "\x1b[44m\x1b[37m";
  } else {
    connection_code = "\x1b[41m\x1b[37m";
  }
  return connection_code;
}

int main(int argc, char** argv)
{
  bool viewmode = false;
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-v") == 0) {
      viewmode = true;
      break;
    }
  }
  ConfigFile config("slowcontrol", (argc > 1) ? argv[1] : "runcontrol");
  TCPSocket socket;
  socket.connect(config.get("RC_GLOBAL_HOST"),  config.getInt("RC_GLOBAL_PORT"));
  TCPSocketWriter writer(socket);
  TCPSocketReader reader(socket);
  writer.writeString("CUI");
  DataObject* data = new DataObject();
  RunStatus* run_status = new RunStatus();
  RunConfig* run_config = new RunConfig();
  run_status->setConfig(run_config);
  std::string name = reader.readString();
  RCMaster* master = new RCMaster(new NSMNode(name), run_config, run_status);
  master->getNode()->setConnection(Connection::ONLINE);
  data->readObject(reader);
  master->setData(data);
  run_status->readObject(reader);
  run_config->readObject(reader);
  if (reader.readBool() == false) {
    viewmode = true;
  }
  for (RCMaster::NSMNodeList::iterator it = master->getNSMNodes().begin();
       it != master->getNSMNodes().end(); it++) {
    NSMNode* node = *it;
    node->setUsed(run_config->getBool(node->getName() + "_used"));
    node->setState(State(run_status->getInt(node->getName() + "_state")));
    node->setConnection(Connection(run_status->getInt(node->getName() + "_connection")));
  }
  ///*
  //data->print();
  //run_status->print();
  //run_config->print();
  //*/
  PThread(new SocketListener(socket, master));
  HIST_ENTRY* history = NULL;
  int nhistory = 0;
  char* prompt = (char*)malloc(name.size() + 3);
  strcpy(prompt, (name + "> ").c_str());
  char* line = NULL;
  std::string operators = "";
  std::string comment = "";
  std::string run_type = "";
  std::vector<std::string> str_v;
  std::string command;
  while (true) {
    if (!viewmode) {
      line = readline(prompt);
      str_v = Belle2::split(line, ' ', 2);
      add_history(line);
      nhistory++;
      if (nhistory >= 20) {
        history = remove_history(0);
        free(history);
      }
      command = Belle2::toupper(str_v[0]);
    }

    if (command == "QUIT") return 0;
    else if (viewmode || command == "STATE") {
      NSMNode* node = master->getNode();
      State state = master->getNode()->getState();
      Connection connection = master->getNode()->getConnection();
      std::string state_code = getStateCode(state);
      std::string connection_code = getConnectionCode(connection);
      if (viewmode) {
        fputs("\033[2J\033[0;0H", stdout);
        rewind(stdout);
        ftruncate(1, 0);
      }
      std::stringstream ss;
      ss << " " << argv[0] << " view only mode: "
         << master->getNode()->getName() << " status " << std::endl
         << std::endl;
      ss << " Updated at " << Date().toString() << std::endl;
      ss << "---------------------------------------------" << std::endl;
      ss << Belle2::form("%15s |  %d  ",
                         "Exp#", master->getStatus()->getExpNumber()) << std::endl;
      ss << Belle2::form("%15s |  %d  ",
                         "Run#", master->getStatus()->getColdNumber()) << std::endl;
      ss << Belle2::form("%15s |  %d  ",
                         "Sub#", master->getStatus()->getHotNumber()) << std::endl;
      ss << Belle2::form("%15s |  %s  ",
                         "Comment", master->getStatus()->getComment().c_str()) << std::endl;
      ss << "---------------------------------------------" << std::endl;
      ss << Belle2::form("%15s |  %10s  |  %10s  ",
                         "Node", "State", "Connection") << std::endl;
      ss << "---------------------------------------------" << std::endl;;
      ss << Belle2::form("%15s | %s %10s \x1b[49m\x1b[39m | %s%10s \x1b[49m\x1b[39m",
                         (node->getName() + " (master)").c_str(),
                         state_code.c_str(), state.getLabel(),
                         connection_code.c_str(), connection.getLabel()) << std::endl;
      for (RCMaster::NSMNodeList::iterator it = master->getNSMNodes().begin();
           it != master->getNSMNodes().end(); it++) {
        node = *it;
        state = node->getState();
        connection = node->getConnection();
        if (!node->isUsed()) {
          state_code = "";
          ss << Belle2::form("%15s |  NOTUSED   ",
                             node->getName().c_str(),
                             state.getLabel(), connection.getLabel()) << std::endl;
        } else {
          state_code = getStateCode(state);
          connection_code = getConnectionCode(connection);
          ss << Belle2::form("%15s | %s %10s \x1b[49m\x1b[39m | %s%10s \x1b[49m\x1b[39m",
                             node->getName().c_str(),
                             state_code.c_str(), state.getLabel(),
                             connection_code.c_str(), connection.getLabel()) << std::endl;
        }
      }
      ss << "---------------------------------------------" << std::endl;
      ss << std::endl;
      fputs(ss.str().c_str(), stdout);
      if (viewmode) {
        sleep(2);
      }
    } else if (command == "INFO") {
      data->print();
      run_status->update();
      run_config->update();
      run_status->print();
      run_config->print();
    } else {
      NSMMessage msg;
      msg.setNParams(3);
      msg.setParam(0, 0);
      msg.setParam(1, 0);
      msg.setParam(2, 0);
      if (command == "TRIGFT") {
        if (str_v.size() >= 4) {
          int trg_mode = 0;
          if (str_v[1] == "aux") trg_mode = 1;
          else if (str_v[1] == "i") trg_mode = 2;
          else if (str_v[1] == "tlu") trg_mode = 3;
          else if (str_v[1] == "pulse") trg_mode = 4;
          else if (str_v[1] == "revo") trg_mode = 5;
          else if (str_v[1] == "random") trg_mode = 6;
          else if (str_v[1] == "possion") trg_mode = 7;
          int dummy_rate = atoi(str_v[2].c_str());
          int trg_limit = atoi(str_v[3].c_str());
          msg.setParam(0, trg_mode);
          msg.setParam(1, dummy_rate);
          msg.setParam(2, trg_limit);
        }
      }
      Command cmd(command.c_str());
      std::string str = (strlen(line) > command.size()) ? (line + command.size() + 1) : "ALL";
      msg.setData(str);
      free(line);
      writer.writeInt(cmd.getId());
      writer.writeObject(msg);
    }
  }
  return 0;
}
