/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck, Matt Barrett                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/SetMergeable.h>
#include <framework/pcore/RootMergeable.h>

#include <TTree.h>
#include <TFile.h>

#include <functional>
#include <string>
#include <unordered_set>
#include <cstdint>

namespace Belle2 {

  class Particle;
  class MCParticle;

  /** Adds the Monte Carlo decay string to a Particle.
   * This is done in the form of a hash, which is currently just an int.
   * The hash can later on be used to get the actual decay string using the DecayHashMap, or using the 'MCDecayString' ntuple tool.
   */

  class ParticleMCDecayStringModule : public Module {
  public:

    /**
     * Constructor
     */
    ParticleMCDecayStringModule();

    /**
     * Initialize the module.
     */
    virtual void initialize() override;

    /**
     * Called for each event.
     */
    virtual void event() override;

    /**
     * Terminate modules
     */
    virtual void terminate() override;

  private:

    /** get the decay string for p. */
    std::string getDecayString(const Particle& p);

    /** get decay string of particle */
    std::string getDecayStringFromParticle(const Particle* p);

    /** get mc decay string from particle */
    std::string getMCDecayStringFromParticle(const Particle* p);

    /** get mc decay string from mc particle */
    std::string getMCDecayStringFromMCParticle(const MCParticle* mcPMatched);

    /** search from mcP upwards for a particle that matches specified mother PDG codes. */
    const MCParticle* getInitialParticle(const MCParticle* mcP);

    /** return decay string for mcPMother, highlight mcPMatched. */
    std::string buildMCDecayString(const MCParticle* mcPMother, const MCParticle* mcPMatched);

    /** Convert the extended string to a more concise format. */
    void convertToConciseString(std::string& string);


  private:

    std::string m_listName; /**< Name of the particle list **/

    std::string m_fileName; /**< Filename in which the hash strings are saved, if empty the strings are not saved */
    std::string m_treeName; /**< Tree name in which the hash strings are saved */

    TFile* m_file; /**< ROOT file to store the hashes and strings */

    StoreObjPtr<RootMergeable<TTree>> m_tree; /**< ROOT TNtuple containting the saved hashes and strings */

    StoreObjPtr<SetMergeable<std::unordered_set<uint64_t>>> m_hashset; /**< Mergeable unordered set containing the encountered hashes */

    const std::string c_ExtraInfoName = "DecayHash"; /**< Name of the extraInfo, which is stored in each Particle **/
    const std::string c_ExtraInfoNameExtended = "DecayHashExtended"; /**< Name of the extraInfo, which is stored in each Particle **/

    float m_decayHash; /**< Decay hash -> The hash of the decay string of the mother particle */
    float m_decayHashExtended; /**< Extended decay hash -> The hash of the decay string of all daughter particles */
    std::string m_decayString; /**< The complete decay string */

    bool m_useConciseString; /**< Switch to use concise format for the extended string. */
    std::string m_identifiers; /**< Characters used to identify particles in the concise decay string format (default: alphabet)." */

    std::hash<std::string> m_hasher; /**< Hash function */
  };

} // Belle2 namespace

