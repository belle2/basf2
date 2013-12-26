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

#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <iostream>

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
          std::cout << name << std::endl;
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
          std::cout << name << std::endl;
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

int main(int, char**)
{
  ConfigFile config("slowcontrol", "runcontrol");
  TCPSocket socket;
  socket.connect(config.get("RC_GLOBAL_HOST"),
                 config.getInt("RC_GLOBAL_PORT"));
  TCPSocketWriter writer(socket);
  TCPSocketReader reader(socket);
  writer.writeString("GUI");
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
  for (RCMaster::NSMNodeList::iterator it = master->getNSMNodes().begin();
       it != master->getNSMNodes().end(); it++) {
    NSMNode* node = *it;
    node->setUsed(run_config->getBool(node->getName() + "_used"));
    node->setState(State(run_status->getInt(node->getName() + "_state")));
    node->setConnection(Connection(run_status->getInt(node->getName() + "_connection")));
  }
  ///*
  data->print();
  run_status->print();
  run_config->print();
  //*/
  PThread(new SocketListener(socket, master));
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
    else if (command == "STATE") {
      NSMNode* node = master->getNode();
      std::cout << Belle2::form("%15s | %8s | %10s ",
                                "Node", "State", "Connection") << std::endl
                << Belle2::form("%15s | %8s | %10s ",
                                (node->getName() + " (master)").c_str(),
                                node->getState().getLabel(),
                                node->getConnection().getLabel()) << std::endl;
      for (RCMaster::NSMNodeList::iterator it = master->getNSMNodes().begin();
           it != master->getNSMNodes().end(); it++) {
        node = *it;
        std::cout << Belle2::form("%15s | %8s | %10s ",
                                  node->getName().c_str(),
                                  node->getState().getLabel(),
                                  node->getConnection().getLabel()) << std::endl;
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
