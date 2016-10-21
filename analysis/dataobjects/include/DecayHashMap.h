/*
 * DecayHashMap.h
 *
 *  Created on: Jul 1, 2014
 *      Author: kronenbitter
 */

#pragma once

#include <vector>
#include <map>
#include <functional>
#include <string>

#include <TObject.h>

namespace Belle2 {

  /**
   * Wrapper to store the the hash <-> decay string combinations of an event
   * Used by ParticleMCDecayStringModule
   */
  class DecayHashMap : public TObject {

  public:
    /** Constructor. */
    DecayHashMap();

    std::string getDecayString(const int hash)
    {
      if (m_HashLookupMap.find(hash) != m_HashLookupMap.end())
        return m_HashLookupMap[hash];
      return 0;
    }

    /** Saves given decayString.
     * Returns the assigned hash (just an int in this case)
     */
    int addDecayHash(const std::string& decayString, bool verbose = false);

  private:
    /** List of already present decay strings. (Not serialized due to funny comment command) */
    std::map<std::string, unsigned int> m_StringLookupMap;
    std::map<unsigned int, std::string> m_HashLookupMap;
    std::hash<std::string> m_hasher; /**< Hash function */

    ClassDef(DecayHashMap,
             2); /**< Wrapper to store the the hash <-> decay string combinations of an event, used by ParticleMCDecayStringModule. */
  };

} // end namespace Belle2

