/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>

#include <arich/dbobjects/ARICHBiasCrateCableMapping.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace Belle2;

ARICHBiasCrateCableMapping::ARICHBiasCrateCableMapping()
{
}

int ARICHBiasCrateCableMapping::getSector(int connectionID) const
{
  if (connectionID < 1) { B2WARNING("ARICHBiasCrateCableMapping::getSector: connectionID " << connectionID << " not valid!"); return 0;}
  std::vector<int> sectorCable = m_connection2cable.find(connectionID)->second;
  return sectorCable[0];
}

int ARICHBiasCrateCableMapping::getCable(int connectionID) const
{
  if (connectionID < 1) { B2WARNING("ARICHBiasCrateCableMapping::getCable: connectionID " << connectionID << " not valid!"); return 0;}
  std::vector<int> sectorCable = m_connection2cable.find(connectionID)->second;
  return sectorCable[1];
}

int ARICHBiasCrateCableMapping::getConnectionID(const std::vector<int>& sectorCable) const
{
  int connectionID = -2;
  for (auto& i : m_connection2cable) {
    if (i.second == sectorCable) connectionID = i.first;
  }
  return connectionID;
}


void ARICHBiasCrateCableMapping::addMapping(int connectionID, std::vector<int> sectorCable)
{
  m_connection2cable.insert(std::pair<int, std::vector<int>>(connectionID, sectorCable));
}

void ARICHBiasCrateCableMapping::print() const
{
  for (auto const& x : m_connection2cable)  {
    int connectionID = x.first;
    std::vector<int> cables = x.second;
    B2INFO("Connection " << connectionID << ": sector " << cables[0] << ", cable " << cables[1]);
  }
}

