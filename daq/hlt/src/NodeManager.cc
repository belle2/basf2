/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "framework/dcore/NodeManager.h"

using namespace Belle2;

NodeManager::NodeManager()
    : m_unitNo(-1), m_nodeNo(-1)
{
  m_nodeinfo = NULL;
  init("NDEF");
}

NodeManager::NodeManager(int unitNo, int nodeNo)
    : m_unitNo(unitNo), m_nodeNo(nodeNo)
{
}

NodeManager::NodeManager(NodeInfo* nodeinfo)
{
  m_nodeinfo = nodeinfo;

  m_unitNo = m_nodeinfo->unitNo();
  m_nodeNo = m_nodeinfo->nodeNo();
}

NodeManager::~NodeManager()
{
  delete m_node;
  delete m_infoSignalMan;
}

int NodeManager::init(const std::string manager)
{
  m_manager = manager;

  return initSignalMan();
}

void NodeManager::setNodeInfo(std::string nodeinfo)
{
  NodeInfo* tmp_node = new NodeInfo();
  tmp_node->deserializedNodeInfo(nodeinfo);
  m_nodeinfo = tmp_node;
  //m_nodeinfo->deserializedNodeInfo (nodeinfo);
  //m_nodeinfo->Print ();
}

void NodeManager::setUnitNo(const int unitNo)
{
}

void NodeManager::setNodeNo(const int nodeNo)
{
}

void NodeManager::broadCasting()
{
  //m_infoSignalMan->broadCasting (m_nodeinfo->serializedNodeInfo ());
  m_infoSignalMan->put(m_nodeinfo->serializedNodeInfo());
}

std::string NodeManager::listen()
{
  return m_infoSignalMan->listening();
}

NodeInfo* NodeManager::nodeInfo()
{
  return m_nodeinfo;
}

int NodeManager::initSignalMan()
{
  if (m_nodeinfo == NULL)
    m_infoSignalMan = new SignalMan(CONTROLPORT, MONITORPORT, m_manager);
  else
    m_infoSignalMan = new SignalMan(CONTROLPORT, MONITORPORT, m_nodeinfo->targetIP());
  // To handle child process
  if (m_infoSignalMan->init() == 1)
    return 1;
  m_infoSignalMan->clearBuffer();
  //m_infoSignalMan->Print ();
}

void NodeManager::Print()
{
  std::cout << "   [NodeManager] ";
  std::cout << "Unit# = " << m_unitNo;
  std::cout << "   Node# = " << m_nodeNo << std::endl;;
  std::cout << "                 ";
  std::cout << "Manager = " << m_manager;
  std::cout << std::endl;
  m_nodeinfo->Print();
}
