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

  init(unit);
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
  //m_eventSeparator->nodeInfo ()->Print ();
  //initWorkerNode (unit);
  //m_workerNodes[0]->nodeInfo ()->Print ();
  //initEventMerger (unit);
}

/* @brief Building NodeInfo object
 * Maybe unnecessary because doing nothing at this moment
 * @param unit UnitInfo object
*/
NodeInfo& UnitManager::buildNodeInfo(UnitInfo& unit)
{
}

/* @brief Broadcasting node information to all nodes inside this unit
*/
void UnitManager::broadCasting(void)
{
  if (m_eventSeparator != NULL)
    m_eventSeparator->broadCasting();
  /*
  for (int i = 0; i < m_workerNodes.size (); i++)
    if (m_workerNodes.size () == 0)
      m_workerNodes[i]->broadCasting ();
  if (m_eventMerger != NULL)
    m_eventMerger->broadCasting ();
    */
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
  nodeinfo->setPortControl(c_InfoInPort);
  nodeinfo->setManagerIP(unit.manager());

  if (type == "ES") {
    nodeinfo->setSourceIP(unit.workerNodes());
    nodeinfo->setTargetIP(unit.eventSeparator());
    nodeinfo->Print();
  } else if (type == "WN")
    nodeinfo->setTargetIP(unit.workerNodes()[nodeNo - 1]);
  else if (type == "EM")
    nodeinfo->setTargetIP(unit.eventMerger());
  else {
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
  for (int i = 0; i < unit.workerNodes().size(); i++) {
    NodeManager* wn = new NodeManager(buildNodeInfo("WN", i + 1, unit));
    wn->init(unit.manager());
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
  m_eventMerger->Print();
  return (m_eventMerger->init(unit.manager()));
}

/* @brief Displaying UnitManager information
 * but do nothing now
*/
void UnitManager::Print()
{
}

