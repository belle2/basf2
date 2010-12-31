/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>
#include <daq/hlt/HLTFramework.h>

using namespace Belle2;

// Constructor for Node (default) or Manager side
HLTFramework::HLTFramework(ENodeType nodeType) : m_nodeType(nodeType)
{
  m_HLTManager = NULL;
}

// Constructor for Manager side
HLTFramework::HLTFramework(ENodeType nodeType, std::string xmlHLTInfo)
    : m_nodeType(nodeType), m_xmlHLTInfo(xmlHLTInfo)
{
  m_HLTProcess = NULL;
}

HLTFramework::~HLTFramework()
{
  delete m_HLTManager;
  delete m_HLTProcess;
}

EStatus HLTFramework::init()
{
  EStatus returnCode;

  if (m_nodeType == c_ProcessNode)
    returnCode = initProcessNode();
  else if (m_nodeType == c_ManagerNode)
    returnCode = initManager();
  else {
    B2ERROR("Unexpected node type");
    returnCode = c_InitFailed;
  }

  return returnCode;
}

EStatus HLTFramework::initProcessNode()
{
  m_nodeManager = new NodeManager();
  m_nodeInfo = m_nodeManager->listen();
  m_nodeManager->setNodeInfo(m_nodeInfo);

  if (m_nodeManager->nodeInfo()->type() == "ES") {
    B2INFO("Assignment as a event separator");
  } else if (m_nodeManager->nodeInfo()->type() == "WN") {
    B2INFO("Assignment as a worker node");
  } else if (m_nodeManager->nodeInfo()->type() == "EM") {
    B2INFO("Assignment as a event merger");
  } else {
    B2ERROR("Wrong node type");
  }

  m_nodeManager->Print();
}

EStatus HLTFramework::initManager()
{
  m_HLTManager = new HLTManager(m_xmlHLTInfo);
  //m_HLTManager->Print ();
  m_HLTManager->broadCasting();
}

EStatus HLTFramework::nodeType(ENodeType nodeType)
{
  m_nodeType = nodeType;

  return c_Success;
}

ENodeType HLTFramework::nodeType()
{
  return m_nodeType;
}

EStatus HLTFramework::xmlHLTInfo(std::string xmlHLTInfo)
{
  if (m_nodeType != c_ManagerNode) {
    B2ERROR("This node is not manager node so it doens't need it");
    return c_FuncError;
  } else {
    m_xmlHLTInfo = xmlHLTInfo;
    return c_Success;
  }
}

EStatus HLTFramework::beginRun()
{
  m_nodeManager->setNodeInfo(m_nodeManager->listen());
}
