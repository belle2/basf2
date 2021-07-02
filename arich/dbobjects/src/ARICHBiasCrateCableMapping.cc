/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>

#include <arich/dbobjects/ARICHBiasCrateCableMapping.h>
#include <iostream>

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

