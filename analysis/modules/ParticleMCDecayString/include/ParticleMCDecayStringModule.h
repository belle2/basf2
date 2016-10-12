/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

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


  private:

    /** get the decay string for p. */
    std::string getDecayString(const Particle& p);

    /** search from mcP upwards for a particle that matches specified mother PDG codes. */
    const MCParticle* getMother(const MCParticle& mcP);

    /** return decay string for mcPMother, highlight mcPMatched. */
    std::string buildDecayString(const MCParticle& mcPMother, const MCParticle& mcPMatched);

  private:

    std::string m_listName; /**< Name of the particle list **/

    std::vector<int> m_motherPDGs; /**< List of potential mother particles **/

    bool m_removeFSR; /**< If true, final state radiation (FSR) photons are removed from the decay string. */

    bool m_printHashes; /**< if true, each new hash will be printed on stdout */

    const std::string c_ExtraInfoName = "DecayHash"; /**< Name of the extraInfo, which is stored in each Particle **/
  };

} // Belle2 namespace

