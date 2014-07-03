#include <analysis/dataobjects/DecayHashMap.h>

#include <stdexcept>

using namespace Belle2;

ClassImp(DecayHashMap)

DecayHashMap::DecayHashMap() : m_LookupIsUpdated(false)
{
  //Performance optimization
  m_DecayStringVector.reserve(20);
}

std::string DecayHashMap::getDecayString(const int hash) const
{
  return m_DecayStringVector.at(hash);
}

void DecayHashMap::updateLookupMap()
{
  for (unsigned int iString = 0; iString < m_DecayStringVector.size(); ++iString) {
    std::string decayString = m_DecayStringVector[iString];
    if (m_StringLookupMap.find(decayString) != m_StringLookupMap.end() && m_StringLookupMap[decayString] != iString) {
      throw std::runtime_error("Error in UpdateLookupMap in DecayHashMap");
    }

    m_StringLookupMap[decayString] = iString;
  }

  m_LookupIsUpdated = true;
}

int DecayHashMap::addDecayHash(const std::string& decayString)
{
  if (not m_LookupIsUpdated) {
    updateLookupMap();
  }

  if (m_StringLookupMap.find(decayString) == m_StringLookupMap.end()) {
    m_DecayStringVector.push_back(decayString);
    return (m_DecayStringVector.size() - 1);
  } else {
    return m_StringLookupMap[decayString];
  }
}

