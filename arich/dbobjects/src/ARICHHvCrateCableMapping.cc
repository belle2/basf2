/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>

#include <arich/dbobjects/ARICHHvCrateCableMapping.h>
#include <iostream>

using namespace std;
using namespace Belle2;

ARICHHvCrateCableMapping::ARICHHvCrateCableMapping()
{
}

int ARICHHvCrateCableMapping::getSector(int connectionID) const
{
  std::vector<int> sectorCable = m_connection2cable.find(connectionID)->second;
  return sectorCable[0];
}

int ARICHHvCrateCableMapping::getCable(int connectionID) const
{
  std::vector<int> sectorCable = m_connection2cable.find(connectionID)->second;
  return sectorCable[1];
}

int ARICHHvCrateCableMapping::getConnectionID(const std::vector<int>& sectorCable) const
{
  int connectionID = -1;
  for (auto& i : m_connection2cable) {
    if (i.second == sectorCable) connectionID = i.first;
  }
  return connectionID;
}


void ARICHHvCrateCableMapping::addMapping(int connectionID, std::vector<int> sectorCable)
{
  m_connection2cable.insert(std::pair<int, std::vector<int>>(connectionID, sectorCable));
}

void ARICHHvCrateCableMapping::print() const
{
  for (auto const& x : m_connection2cable)  {
    int connectionID = x.first;
    std::vector<int> cables = x.second;
    B2INFO("Connection " << connectionID << ": sector " << cables[0] << ", cable " << cables[1]);
  }
}

