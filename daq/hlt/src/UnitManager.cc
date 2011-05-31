/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/UnitManager.h>

using namespace Belle2;

/* @brief UnitManager constructor
 * @param unit UnitInfo object
*/
UnitManager::UnitManager(UnitInfo& unit)
{
  m_unitNo = unit.unitNo();
  m_WNs = unit.workerNodes().size();

  m_eventSeparator = NULL;
  m_workerNodes.clear();
  m_eventMerger = NULL;
}

/* @brief UnitManager destructor
*/
UnitManager::~UnitManager()
{
}

/* @brief Initializing UnitManager
 * @param unit UnitInfo object
 * @return c_Success Initialization success
 * @reuurn c_InitFailed Initialization failed
*/
EStatus UnitManager::init(UnitInfo& unit)
{
  if (initEventSeparator(unit) != c_Success)
    return c_InitFailed;
  if (initEventMerger(unit) != c_Success)
    return c_InitFailed;
  if (initWorkerNode(unit) != c_Success)
    return c_InitFailed;

  return c_Success;
}

/* @brief Broadcasting node information to all nodes inside this unit
*/
EStatus UnitManager::broadCasting(void)
{
  EStatus returnCode;

  if (m_eventSeparator != NULL) {
    returnCode = m_eventSeparator->broadCasting();
    if (returnCode != c_Success)
      return returnCode;
  }
  for (unsigned int i = 0; i < m_workerNodes.size(); i++) {
    if (m_workerNodes[i] != NULL) {
      returnCode = m_workerNodes[i]->broadCasting();
      if (returnCode != c_Success)
        return returnCode;
    }
  }
  if (m_eventMerger != NULL) {
    returnCode = m_eventMerger->broadCasting();
    if (returnCode != c_Success)
      return returnCode;
  }

  return c_Success;
}

/* @brief Building NodeInfo object
 * @param type Type of the node
 * @param nodeNo Node number of the node
 * @param unit UnitInfo object
 * @return NodeInfo object built
*/
NodeInfo* UnitManager::buildNodeInfo(const std::string type, const int nodeNo, UnitInfo& unit)
{
  NodeInfo* nodeinfo = new NodeInfo(type, unit.unitNo(), nodeNo);
  //nodeinfo->setPortData(c_DataInPort, c_DataOutPort);
  nodeinfo->setPortControl(c_InfoInPort);
  nodeinfo->setManagerIP(unit.manager());
  nodeinfo->setSteeringName(unit.steering());

  if (type == "ES") {
    B2INFO("Building NodeInfo for ES (" << unit.eventSeparator() << ")...");
    nodeinfo->setThisIP(unit.eventSeparator());
    nodeinfo->setTargetIP(unit.workerNodes());
    nodeinfo->setPortData(c_DataInPort, c_DataOutPort);
    nodeinfo->Print();
  } else if (type == "WN") {
    B2INFO("Building NodeInfo for WN (" << unit.workerNodes()[nodeNo - 1] << ")...");
    nodeinfo->setThisIP(unit.workerNodes()[nodeNo - 1]);
    nodeinfo->setTargetIP(unit.eventMerger());
    nodeinfo->setPortData(c_DataOutPort, c_DataInPort);
    nodeinfo->Print();
  } else if (type == "EM") {
    B2INFO("Building NodeInfo for EM (" << unit.eventMerger() << ")...");
    nodeinfo->setThisIP(unit.eventMerger());
    nodeinfo->setTargetIP("");
    nodeinfo->setPortData(c_DataInPort, c_DataOutPort);
    nodeinfo->Print();
  } else {
    B2ERROR("Wrong node type!");
    return NULL;
  }

  return nodeinfo;
}

/* @brief Initializing NodeManager for event separator
 * Event separator should have node # 0 all the time
 * @param unitinfo UnitInfo object
 * @return c_Success Initialization success
 * @return c_InitFailed Initialization failed
*/
EStatus UnitManager::initEventSeparator(UnitInfo& unit)
{
  m_eventSeparator = new NodeManager(buildNodeInfo("ES", 0, unit));
  return (m_eventSeparator->init(unit.manager()));
}

/* @brief Initializing NodeManager for workernodes
 * @param unitinfo UnitInfo object
 * @return c_Success Initialization success
 * @return c_InitFailed Initialization failed
*/
EStatus UnitManager::initWorkerNode(UnitInfo& unit)
{
  for (unsigned int i = 0; (int)i < m_WNs; i++) {
    NodeManager* wn = new NodeManager(buildNodeInfo("WN", i + 1, unit));
    if (wn->init(unit.manager()) != c_Success)
      return c_InitFailed;
    m_workerNodes.push_back(wn);
  }

  return c_Success;
}

/* @brief Initializing NodeManager for event merger
 * Event merger should have something for node #, 100 for now, arbitrary
 * @param unitinfo UnitInfo object
 * @return c_Success Initialization success
 * @return c_InitFailed Initialization failed
*/
EStatus UnitManager::initEventMerger(UnitInfo& unit)
{
  m_eventMerger = new NodeManager(buildNodeInfo("EM", 100, unit));
  //m_eventMerger->Print();
  return (m_eventMerger->init(unit.manager()));
}

/* @brief Displaying UnitManager information
 * but do nothing now
*/
void UnitManager::Print()
{
  B2INFO("   unit#     = " << m_unitNo);
  B2INFO("   # of WNs  = " << m_WNs);
  B2INFO("   ES        = " << m_eventSeparator->nodeInfo()->thisIP());
}

