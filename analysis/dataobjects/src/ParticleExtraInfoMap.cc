#include <analysis/dataobjects/ParticleExtraInfoMap.h>

//#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(ParticleExtraInfoMap);

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
    try {
      if (m_maps[iMap].at(name) == insertIndex) {
        return iMap;
      }
    } catch (...) {
      //not found, ok.
    }
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
    try {
      if (map.at(name) == insertIndex) {
        //seems promising
        if (compatible(oldMap, map, insertIndex)) {
          //compatible with oldMap, can be used
          return iMap;
        }
      }
    } catch (...) {
      continue; //some key wasn't found, got to next map
    }
  }

  //nothing found, add new map (copy all entries prior to insertIndex from oldMap)
  IndexMap map;
  for (const auto & pair : oldMap) {
    if (pair.second < insertIndex)
      map[pair.first] = pair.second;
  }
  map[name] = insertIndex;
  m_maps.push_back(map);

  //return new index
  return m_maps.size() - 1;
}

bool ParticleExtraInfoMap::compatible(const IndexMap& oldMap, const IndexMap& map, unsigned int insertIndex)
{
  for (const auto & pair : oldMap) {
    if (pair.second < insertIndex) {
      if (map.at(pair.first) != pair.second) {
        //mismatch
        return false;
      }
    }
  }
  return true;
}
