/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/NodeManager.h>

using namespace Belle2;

/* @brief NodeManager constructor
 * If there is no parameter, a null NodeManager is created and initialized
*/
NodeManager::NodeManager()
    : m_unitNo(-1), m_nodeNo(-1)
{
  m_nodeinfo = NULL;
}

/* @brief NodeManager constructor
 * Initializing member variables (Is this constructor really needed?)
*/
NodeManager::NodeManager(int unitNo, int nodeNo)
    : m_unitNo(unitNo), m_nodeNo(nodeNo)
{
}

/* @brief NodeManager constructor
 * Initializing NodeManager with NodeInfo object
 * @param nodeinfo NodeInfo object containing the individual node information
*/
NodeManager::NodeManager(NodeInfo* nodeinfo)
{
  m_nodeinfo = nodeinfo;

  m_unitNo = m_nodeinfo->unitNo();
  m_nodeNo = m_nodeinfo->nodeNo();
}

/* @brief NodeManager destructor
*/
NodeManager::~NodeManager()
{
  delete m_nodeinfo;
  delete m_infoSignalMan;
}

/* @brief NodeManager initializer
 * @param manager IP address of manager node
 * @return c_Success Initialization of SignalMan success
 * @return c_InitFailed Initialization of SignalMan failed
*/
EStatus NodeManager::init(const std::string manager)
{
  m_manager = manager;

  return initSignalMan();
}

/* @brief Reconstruct NodeInfo object from serialized one
 * @param nodeinfo Serialized NodeInfo object
*/
void NodeManager::setNodeInfo(std::string nodeinfo)
{
  NodeInfo* tmp_node = new NodeInfo();
  tmp_node->deserializedNodeInfo(nodeinfo);
  m_nodeinfo = tmp_node;
}

/* @brief Initialize EvtSender only
 * This is for the beginning of process node
 * @return c_Success Initialization success
 * @return c_InitFailed Initialization failed
 * @return c_TermCalled Termination of EvtSender
*/
EStatus NodeManager::initEvtSender()
{
  if (m_nodeinfo == NULL)
    return c_InitFailed;
  else
    return m_infoSignalMan->runEvtSender();
}

/* @brief Set unit number of this node
 * @param unitNo Unit number of this node
*/
void NodeManager::setUnitNo(const int unitNo)
{
  m_unitNo = unitNo;
}

/* @brief Set node number of this node
 * @param nodeNo Node number of this node
*/
void NodeManager::setNodeNo(const int nodeNo)
{
  m_nodeNo = nodeNo;
}

void NodeManager::steeringReconstructor()
{
  //m_nodeinfo->getSteering ();
}

/* @brief Put data into outgoing FIFO after serialization
*/
void NodeManager::broadCasting()
{
  m_infoSignalMan->put(m_nodeinfo->serializedNodeInfo());
}

/* @brief Get data from incoming FIFO as serialized one
 * @return Serialized data transferred
*/
std::string NodeManager::listen()
{
  B2INFO("Retreiving node infomation from manager node...");
  return m_infoSignalMan->get();
}

/* @brief Get NodeInfo object
 * @return Pointer to NodeInfo object
*/
NodeInfo* NodeManager::nodeInfo()
{
  return m_nodeinfo;
}

/* @brief Initializing SignalMan for the communication
 * @return c_Success Initialization of SignalMan success (EvtSender and EvtReceiver never return this)
 * @return c_InitFailed Initialization of SignalMan failed (SignalMan never return this)
 * @return c_TermCalled If EvtSender or EvtReceiver get the termination code, it returns this
*/
EStatus NodeManager::initSignalMan()
{
  // For node mode, incoming port should be a control line and outgoing port should be a monitor line
  if (m_nodeinfo == NULL)
    m_infoSignalMan = new SignalMan(c_ControlPort, c_MonitorPort, m_manager);
  // For manager mode, incoming port should be a monitor line and outgoing port should be a control line
  else
    m_infoSignalMan = new SignalMan(c_MonitorPort, c_ControlPort, m_nodeinfo->targetIP());

  m_infoSignalMan->init("B2ControlIn", "B2ControlOut");

  // If there is no NodeInfo object, that is, a process node, runs EvtReceiver first because it doesn't know
  // the IP address of manager node
  if (m_nodeinfo == NULL)
    return m_infoSignalMan->runEvtReceiver();
  else
    return m_infoSignalMan->doCommunication();
}

/* @brief Test if this process is EvtSender or not
 * @return 1 if the process is EvtSender
 * @return 0 if the process is not EvtSender
*/
int NodeManager::isEvtSender()
{
  return m_infoSignalMan->isEvtSender();
}

/* @brief Test if this process is EvtReceiver or not
 * @return 1 if the process is EvtReceiver
 * @return 0 if the process is not EvtReceiver
*/
int NodeManager::isEvtReceiver()
{
  return m_infoSignalMan->isEvtReceiver();
}

/* @brief Displaying information of NodeManager (only for debugging)
*/
void NodeManager::Print()
{
  B2INFO("   [NodeManager] ");
  B2INFO("Unit# = " << m_unitNo);
  B2INFO("   Node# = " << m_nodeNo);
  B2INFO("                 ");
  B2INFO("Manager = " << m_manager);
  m_nodeinfo->Print();
}
