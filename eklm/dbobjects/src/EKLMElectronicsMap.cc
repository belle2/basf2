/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMElectronicsMap.h>
#include <eklm/dataobjects/EKLMElementNumbers.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMElectronicsMap::EKLMElectronicsMap()
{
}

EKLMElectronicsMap::~EKLMElectronicsMap()
{
}

const int* EKLMElectronicsMap::getSectorByLane(
  EKLMDataConcentratorLane* lane) const
{
  std::map<EKLMDataConcentratorLane, int>::const_iterator it;
  it = m_MapLaneSector.find(*lane);
  if (it == m_MapLaneSector.end())
    return nullptr;
  return &(it->second);
}

const EKLMDataConcentratorLane* EKLMElectronicsMap::getLaneBySector(
  int sector) const
{
  std::map<int, EKLMDataConcentratorLane>::const_iterator it;
  it = m_MapSectorLane.find(sector);
  if (it == m_MapSectorLane.end())
    return nullptr;
  return &(it->second);
}

void EKLMElectronicsMap::addSectorLane(
  int endcap, int layer, int sector, int copper, int dataConcentrator, int lane)
{
  static const EKLMElementNumbers elementNumbers;
  int sectorGlobal;
  EKLMDataConcentratorLane laneId;
  sectorGlobal = elementNumbers.sectorNumber(endcap, layer, sector);
  laneId.setCopper(copper);
  laneId.setDataConcentrator(dataConcentrator);
  laneId.setLane(lane);
  if (m_MapSectorLane.find(sectorGlobal) != m_MapSectorLane.end()) {
    B2ERROR("Sector already exists in the electronics map."
            << LogVar("Global sector number", sectorGlobal)
            << LogVar("Endcap", endcap) << LogVar("Layer", layer)
            << LogVar("Sector", sector));
    return;
  }
  if (m_MapLaneSector.find(laneId) != m_MapLaneSector.end()) {
    B2ERROR("Lane already exists in the electronics map."
            << LogVar("Copper", copper)
            << LogVar("Data concentrator", dataConcentrator)
            << LogVar("Lane", lane));
    return;
  }
  m_MapSectorLane.insert(std::pair<int, EKLMDataConcentratorLane>(
                           sectorGlobal, laneId));
  m_MapLaneSector.insert(std::pair<EKLMDataConcentratorLane, int>(
                           laneId, sectorGlobal));
}

