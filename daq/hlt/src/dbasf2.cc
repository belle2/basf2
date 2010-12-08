/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>

#include "framework/dcore/styledef.h"
#include "framework/dcore/HLTManager.h"
#include "framework/dcore/NodeManager.h"
#include "framework/dcore/EventSeparator.h"
#include "framework/dcore/WorkerNode.h"
#include "framework/dcore/EventMerger.h"

using namespace Belle2;

void usage();
void usageInteractive();
void showLogo(const std::string);
void debugMessage(const std::string);
void modeNode(int nproc = 1);
void modeManager(char*);
void modeInteractive();
void interprete(const std::string);
void cmdTokenizer(const std::string&, std::vector<std::string>&);

int main(int argc, char** argv)
{
  int nproc = 1;

  if (argc == 3) {
    std::string mode = argv[1];

    if (mode == "Manager") {
      showLogo("Manager");
      modeManager(argv[2]);
    } else {
      usage();
      return 0;
    }
  } else if (argc == 2) {
    std::string mode = argv[1];

    if (mode == "Node") {
      showLogo("Node");
      modeNode(nproc);
    }
  } else if (argc == 1) {
    showLogo("Interactive");
    modeInteractive();
  } else {
    usage();
    return 0;
  }

  std::cout << "Press Enter.." << std::endl;
  std::cin;

  return 0;
}

void usage()
{
  std::cout << "Usage: dbasf2 Node" << std::endl;
  std::cout << "       dbasf2 Manager {HLT information}" << std::endl;
}

void usageInteractive()
{
  std::cout << "    Node" << std::endl;
  std::cout << "    Manager [HLT information]" << std::endl;
}

void showLogo(const std::string mode)
{
  std::cout << __GREEN << "**********************************************************" << __END << std::endl;
  std::cout << __GREEN << " D(ummy)BASF2 " << __YELLOW << "v0.1" << __END;
  std::cout << __GREEN << "     by Soohyung Lee (Korea University)" << __END << std::endl;
  std::cout << __GREEN << " " << __YELLOW << mode << " mode" << __END << std::endl;
  std::cout << __GREEN << "**********************************************************" << __END << std::endl;
}

void debugMessage(const std::string msg)
{
  std::cout << __WHITE << "[DEBUG] " << msg << __END << std::endl;
}

void modeNode(int nproc)
{
  //__________________________________________________Node manager deployment
  NodeManager* nodeManager = new NodeManager();
  if (nproc <= 0)
    nproc = 1;
  //nodeManager->init (manager);
  //debugMessage ("Node manager allocated and initialized");
  //debugMessage ("(This also implies the allocation and initialization of SignalMan inside)");
  while (1) {
    std::string nodeinfo = nodeManager->listen();

    nodeManager->setNodeInfo(nodeinfo);

    if (nodeManager->nodeInfo()->type() == "TERM")
      break;

    Node* thisNode;

    if (nodeManager->nodeInfo()->type() == "ES") {
      std::cout << "I'm an event separator!" << std::endl;
      thisNode = new Node(nproc);
      thisNode->init(nodeManager->nodeInfo());
      //thisNode = new EventSeparator (nproc);
    } else if (nodeManager->nodeInfo()->type() == "WN") {
      std::cout << "I'm a worker node!" << std::endl;
      //thisNode = new WorkerNode ();
    } else if (nodeManager->nodeInfo()->type() == "EM") {
      std::cout << "I'm an event merger!" << std::endl;
      //thisNode = new EventMerger ();
    } else {
      std::cout << "Wrong assignment of a node!" << std::endl;
    }
  }
}

void modeManager(char* input)
{
  debugMessage("HLT manager allocated and initialized");
  HLTManager* hltManager = new HLTManager(input);
  hltManager->broadCasting();
}

void modeInteractive()
{
  std::string cmd = "";
  while (1) {
    std::cout << ">> ";
    std::getline(std::cin, cmd, '\n');
    interprete(cmd);
  }
}

void interprete(const std::string cmd)
{
  std::vector<std::string> cmds;
  cmds.clear();

  cmdTokenizer(cmd, cmds);

  if (cmds.size() > 0 && cmds[0] == "mode") {
    if (cmds.size() > 1 && cmds[1] == "Node") {
      modeNode();
    }
    if (cmds.size() > 1 && cmds[1] == "Manager") {
      if (cmds.size() > 2)
        modeManager((char*)cmds[2].c_str());
    }
  } else if (cmds.size() > 0 && cmds[0] == "help") {
    usageInteractive();
  } else if (cmds.size() > 0 && cmds[0] == "exit")
    exit(0);
  else if (cmds.size() == 0) {
  } else {
    std::cout << "unknown command: " << cmds[0] << std::endl;
  }
}

void cmdTokenizer(const std::string& cmd, std::vector<std::string>& cmds)
{
  std::string::size_type startPosition = cmd.find_first_not_of(" ", 0);
  std::string::size_type first = cmd.find_first_of(" ", startPosition);

  while (std::string::npos != first || std::string::npos != startPosition) {
    cmds.push_back(cmd.substr(startPosition, first - startPosition));
    startPosition = cmd.find_first_not_of(" ", first);
    first = cmd.find_first_of(" ", startPosition);
  }
}
