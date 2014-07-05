/*
 * DecayHashMap.h
 *
 *  Created on: Jul 1, 2014
 *      Author: kronenbitter
 */

#pragma once

#include <vector>
#include <map>

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

    /** Return decay string matching this hash.
     *
     * throws std::out_of_range if this hash doesn't exist
     */
    std::string getDecayString(const int hash) const;

    /** Saves given decayString.
     * Returns the assigned hash (just an int in this case)
     */
    int addDecayHash(const std::string& decayString);

  private:
    /** rebuild m_StringLookupMap. */
    void updateLookupMap();

  private:
    std::vector<std::string> m_DecayStringVector; /**< map variable names to values. */

    /** List of already present decay strings. (Not serialized due to funny comment command) */
    std::map<std::string, unsigned int> m_StringLookupMap; //!

    /** is m_StringLookupMap up-to-date?. */
    bool m_LookupIsUpdated; //!

    ClassDef(DecayHashMap, 2); /**< Wrapper to store the the hash <-> decay string combinations of an event, used by ParticleMCDecayStringModule. */
  };

} // end namespace Belle2

