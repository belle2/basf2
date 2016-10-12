#include <analysis/dataobjects/DecayHashMap.h>

#include <stdexcept>
#include <iostream>

using namespace Belle2;

DecayHashMap::DecayHashMap()
{

}



int DecayHashMap::addDecayHash(const std::string& decayString, bool verbose)
{
  if (m_StringLookupMap.find(decayString) == m_StringLookupMap.end()) {
    const unsigned int hash = m_hasher(decayString);
    m_StringLookupMap[decayString] = hash;
    m_HashLookupMap[hash] = decayString;
    if (verbose) {
      std::cout << "FOUND_NEW_DECAY_HASH " << hash << " " << decayString << std::endl;
    }
  }
  return m_StringLookupMap[decayString];
}

