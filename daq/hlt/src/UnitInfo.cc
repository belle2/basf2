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

#include "framework/dcore/UnitInfo.h"

using namespace Belle2;

UnitInfo::UnitInfo()
{
}

UnitInfo::UnitInfo(const int unitNo)
    : m_unitNo(unitNo)
{
  m_workerNodes.clear();
}

UnitInfo::~UnitInfo()
{
}

void UnitInfo::eventSeparator(const std::string ES)
{
  m_eventSeparator = ES;
}

void UnitInfo::workerNodes(const std::string WN)
{
  m_workerNodes.push_back(WN);
}

void UnitInfo::workerNodes(const std::vector<std::string> WNs)
{
  m_workerNodes = WNs;
}

void UnitInfo::eventMerger(const std::string EM)
{
  m_eventMerger = EM;
}

void UnitInfo::manager(const std::string MAN)
{
  m_manager = MAN;
}

std::string UnitInfo::eventSeparator()
{
  return m_eventSeparator;
}

std::vector<std::string> UnitInfo::workerNodes()
{
  return m_workerNodes;
}

std::string UnitInfo::eventMerger()
{
  return m_eventMerger;
}

std::string UnitInfo::manager()
{
  return m_manager;
}

int UnitInfo::unitNo()
{
  return m_unitNo;
}

void UnitInfo::Print()
{
  std::cout << "unitNo = " << m_unitNo << std::endl;
  std::cout << "manager = " << m_manager << std::endl;
  std::cout << "eventSeparator = " << m_eventSeparator << std::endl;
  std::cout << "eventMerger = " << m_eventMerger << std::endl;
}
