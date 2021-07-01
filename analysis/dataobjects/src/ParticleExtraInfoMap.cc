/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/dataobjects/ParticleExtraInfoMap.h>


using namespace Belle2;

unsigned int ParticleExtraInfoMap::getIndex(unsigned int mapID, const std::string& name) const
{
  const IndexMap& map = m_maps[mapID];

  auto it = map.find(name);
  if (it == map.end())
    return 0;
  else
    return it->second;
}

unsigned int ParticleExtraInfoMap::getMapForNewVar(const std::string& name)
{
  const unsigned int insertIndex = 1; //0 reserved
  for (unsigned int iMap = 0; iMap < m_maps.size(); iMap++) {
    const auto it = m_maps[iMap].find(name);
    if (it != m_maps[iMap].end() and it->second == insertIndex)
      return iMap;
  }
  //nothing found, add new map
  IndexMap map;
  map[name] = insertIndex;
  m_maps.push_back(map);

  //return new index
  return m_maps.size() - 1;
}

unsigned int ParticleExtraInfoMap::getMapForNewVar(const std::string& name, unsigned int oldMapID, unsigned int insertIndex)
{
  const IndexMap& oldMap = m_maps[oldMapID];

  //first check if old map can be reused
  const unsigned int lastIndexInOldMap = oldMap.size(); //+1 because of reserved 0 index
  if (lastIndexInOldMap + 1 == insertIndex) {
    //we can make oldMap fit by adding one entry
    m_maps[oldMapID][name] = insertIndex;
    return oldMapID;
  }
  auto oldMapIter = oldMap.find(name);
  if (oldMapIter != oldMap.end()) {
    if (oldMapIter->second == insertIndex) {
      return oldMapID; //nothing to do
    }
  }

  for (unsigned int iMap = 0; iMap < m_maps.size(); iMap++) {
    const IndexMap& map = m_maps[iMap];
    const auto it = map.find(name);
    if (it != map.end() and it->second == insertIndex) {
      //seems promising
      if (isCompatible(oldMap, map, insertIndex)) {
        //compatible with oldMap, can be used
        return iMap;
      }
    }
  }

  //nothing found, add new map (copy all entries prior to insertIndex from oldMap)
  IndexMap map;
  for (const auto& pair : oldMap) {
    if (pair.second < insertIndex)
      map[pair.first] = pair.second;
  }
  map[name] = insertIndex;
  m_maps.push_back(map);

  //return new index
  return m_maps.size() - 1;
}

bool ParticleExtraInfoMap::isCompatible(const IndexMap& oldMap, const IndexMap& map, unsigned int insertIndex)
{
  for (const auto& pair : oldMap) {
    if (pair.second < insertIndex) {
      const auto it = map.find(pair.first);
      if (it == map.end() or it->second != pair.second) {
        //mismatch
        return false;
      }
    }
  }
  return true;
}
