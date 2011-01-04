/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/HLTFramework.h>

using namespace Belle2;

/* @brief HLTFramework constructor
 * @param nodeType Type of this node
*/
HLTFramework::HLTFramework(ENodeType nodeType) : m_nodeType(nodeType)
{
  m_HLTManager = NULL;
}

/* @brief HLTFramework constructor
 * @param nodeType Type of this node
 * @param xmlHLTInfo Input XML file of HLT farm
*/
HLTFramework::HLTFramework(ENodeType nodeType, std::string xmlHLTInfo)
    : m_nodeType(nodeType), m_xmlHLTInfo(xmlHLTInfo)
{
  m_HLTProcess = NULL;
}

/* @brief HLTFramework destructor
*/
HLTFramework::~HLTFramework()
{
  if (m_nodeType == c_ManagerNode) {
    B2INFO("Manager node terminates...");
    delete m_HLTManager;
  } else if (m_nodeType == c_ProcessNode) {
    B2INFO("Process node terminates...");
    delete m_HLTProcess;
    delete m_nodeManager;
  }
}

/* @brief Initializing HLTFramework
 * @return c_Success Initializing success
 * @return c_InitFailed Initializing failed
*/
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

/* @brief Initializing HLTFramework as a process node
 * @return c_Success Initializing success
 * @return c_InitFailed Initializing failed
*/
EStatus HLTFramework::initProcessNode()
{
  m_nodeManager = new NodeManager();
  EStatus initCode = m_nodeManager->init("NDEF");

  // For the framework itself because forked EvtSender and EvtReceiver never return c_Success
  if (initCode == c_Success) {
    m_nodeManager->setNodeInfo(m_nodeManager->listen());

    EStatus returnCode = m_nodeManager->initEvtSender();
    if (returnCode != c_Success)
      return returnCode;

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

    return c_Success;
  }
  // For forked components (EvtSender and EvtReceiver)
  else if (initCode == c_TermCalled) {
    return c_TermCalled;
  } else {
    return c_InitFailed;
  }
}

/* @brief Initializing HLTFramework as a manager node
 * @return c_Success Initializing success
 * @return c_InitFailed Initializing failed
*/
EStatus HLTFramework::initManager()
{
  m_HLTManager = new HLTManager(m_xmlHLTInfo);
  m_HLTManager->broadCasting();

  return c_Success;
}

/* @brief Set node type of this node
 * @param nodeType Node type of this node
 * @return c_Success Whenever at this moment
*/
EStatus HLTFramework::nodeType(ENodeType nodeType)
{
  m_nodeType = nodeType;

  return c_Success;
}

/* @brief Get node type of this node
 * @return Node type of this node
*/
ENodeType HLTFramework::nodeType()
{
  return m_nodeType;
}

/* @brief Set XML file of HLT information
 * @param File name of XML file of HLT information
 * @return c_Success Setting success
 * @return c_FuncError When the file assigned to process node (it doesn't need it)
*/
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
  m_HLTProcess = new HLTProcess(m_nodeManager->nodeInfo());
  m_HLTProcess->init();
  return m_HLTProcess->beginRun();
}
