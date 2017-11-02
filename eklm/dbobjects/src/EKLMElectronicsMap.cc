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
  struct EKLM::DataConcentratorLane* lane) const
{
  std::map<struct EKLM::DataConcentratorLane, int,
        EKLM::DataConcentratorLaneComparison>::const_iterator it;
  it = m_MapLaneSector.find(*lane);
  if (it == m_MapLaneSector.end())
    return NULL;
  return &(it->second);
}

const struct EKLM::DataConcentratorLane* EKLMElectronicsMap::getLaneBySector(
  int sector) const {
  std::map<int, struct EKLM::DataConcentratorLane>::const_iterator it;
  it = m_MapSectorLane.find(sector);
  if (it == m_MapSectorLane.end())
    return NULL;
  return &(it->second);
}

void EKLMElectronicsMap::addSectorLane(
  int endcap, int layer, int sector, int copper, int dataConcentrator, int lane)
{
  static const EKLMElementNumbers elementNumbers;
  int sectorGlobal;
  struct EKLM::DataConcentratorLane laneId;
  sectorGlobal = elementNumbers.sectorNumber(endcap, layer, sector);
  laneId.copper = copper;
  laneId.dataConcentrator = dataConcentrator;
  laneId.lane = lane;
  if (m_MapSectorLane.find(sectorGlobal) != m_MapSectorLane.end()) {
    B2ERROR("Sector with global number " << sectorGlobal <<
            "(endcap " << endcap << ", layer " << layer << ", sector " <<
            sector << ") already exists in the electronics map.");
    return;
  }
  if (m_MapLaneSector.find(laneId) != m_MapLaneSector.end()) {
    B2ERROR("Lane with data concentrator number " << dataConcentrator <<
            ", lane number " << lane <<
            " already exists in the electronics map.");
    return;
  }
  m_MapSectorLane.insert(std::pair<int, struct EKLM::DataConcentratorLane>(
                           sectorGlobal, laneId));
  m_MapLaneSector.insert(std::pair<struct EKLM::DataConcentratorLane, int>(
                           laneId, sectorGlobal));
}

