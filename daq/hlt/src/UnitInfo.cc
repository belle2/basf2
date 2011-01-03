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

#include <daq/hlt/UnitInfo.h>

using namespace Belle2;

/* @brief UnitInfo constructor
*/
UnitInfo::UnitInfo()
{
}

/* @brief UnitInfo constructor
 * @param unitNo Unit number of this unit
*/
UnitInfo::UnitInfo(const int unitNo)
    : m_unitNo(unitNo)
{
  m_workerNodes.clear();
}

/* @brief UnitInfo destructor
*/
UnitInfo::~UnitInfo()
{
}

/* @brief Set event separator node
 * @param ES IP address of event separator
*/
void UnitInfo::eventSeparator(const std::string ES)
{
  m_eventSeparator = ES;
}

/* @brief Add worker node into the container which contains IP addresses of worker nodes
 * @param WN IP address of worker node
*/
void UnitInfo::workerNodes(const std::string WN)
{
  m_workerNodes.push_back(WN);
}

/* @brief Set worker nodes
 * @param WNs Vector container which contains IP addresses of worker nodes
*/
void UnitInfo::workerNodes(const std::vector<std::string> WNs)
{
  m_workerNodes = WNs;
}

/* @brief Set event merger node
 * @param EM IP address of event merger
*/
void UnitInfo::eventMerger(const std::string EM)
{
  m_eventMerger = EM;
}

/* @brief Set manager node
 * @param MAN IP address of manager node
*/
void UnitInfo::manager(const std::string MAN)
{
  m_manager = MAN;
}

/* @brief Get event separator node
 * @return IP address of event separator
*/
std::string UnitInfo::eventSeparator()
{
  return m_eventSeparator;
}

/* @brief Get worker nodes
 * @return Vector container which contains IP addresses of worker nodes
*/
std::vector<std::string> UnitInfo::workerNodes()
{
  return m_workerNodes;
}

/* @brief Get event merger
 * @return IP address of event merger
*/
std::string UnitInfo::eventMerger()
{
  return m_eventMerger;
}

/* @brief Get manager node
 * @return IP address of manager node
*/
std::string UnitInfo::manager()
{
  return m_manager;
}

/* @brief Get unit number of this unit
 * @return Unit number of this unit
*/
int UnitInfo::unitNo()
{
  return m_unitNo;
}

/* @brief Displaying unit information of this unit (only for debugging)
*/
void UnitInfo::Print()
{
  B2INFO("=================================================");
  B2INFO(" UnitInfo Summary");
  B2INFO("   unitNo = " << m_unitNo);
  B2INFO("   manager = " << m_manager);
  B2INFO("   eventSeparator = " << m_eventSeparator);
  B2INFO("   eventMerger = " << m_eventMerger);
  B2INFO("=================================================");
}
