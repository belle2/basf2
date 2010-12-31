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
#include <framework/logging/Logger.h>

using namespace Belle2;

UnitManager::UnitManager(UnitInfo& unit)
{
  m_unitNo = unit.unitNo();
  m_WNs = unit.workerNodes().size();

  m_eventSeparator = NULL;
  m_workerNodes.clear();
  m_eventMerger = NULL;

  init(unit);
}

UnitManager::~UnitManager()
{
  //delete m_eventSeparator;
  //delete m_eventMerger;
}

int UnitManager::init(UnitInfo& unit)
{
  if (initEventSeparator(unit) == 1)
    return 1;
  //m_eventSeparator->nodeInfo ()->Print ();
  //initWorkerNode (unit);
  //m_workerNodes[0]->nodeInfo ()->Print ();
  //initEventMerger (unit);
}

NodeInfo& UnitManager::buildNodeInfo(UnitInfo& unit)
{
}

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

///* Event separator should have node # 0 all the time
int UnitManager::initEventSeparator(UnitInfo& unit)
{
  m_eventSeparator = new NodeManager(buildNodeInfo("ES", 0, unit));
  //m_eventSeparator = new NodeManager (0, unit.unitNo ());
  return (m_eventSeparator->init(unit.manager()));
}

int UnitManager::initWorkerNode(UnitInfo& unit)
{
  for (int i = 0; i < unit.workerNodes().size(); i++) {
    NodeManager* wn = new NodeManager(buildNodeInfo("WN", i + 1, unit));
    //NodeManager* wn = new NodeManager (i + 1, m_unitNo);
    wn->init(unit.manager());
    m_workerNodes.push_back(wn);
  }
}

///* Event merger should have something for node #, 100 for now, arbitrary
int UnitManager::initEventMerger(UnitInfo& unit)
{
  m_eventMerger = new NodeManager(buildNodeInfo("EM", 100, unit));
  m_eventMerger->Print();
  //m_eventMerger = new NodeManager (100, m_unitNo);
  m_eventMerger->init(unit.manager());
}

void UnitManager::Print()
{
}

