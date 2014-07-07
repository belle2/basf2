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
     * Destructor
     */
    virtual ~ParticleMCDecayStringModule();

    /**
     * Initialize the module.
     */
    virtual void initialize();

    /**
     * Called when a new run is started.
     */
    virtual void beginRun();

    /**
     * Called for each event.
     */
    virtual void event();

    /**
     * Called when run ended.
     */
    virtual void endRun();

    /**
     * Terminates the module.
     */
    virtual void terminate();

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

    const std::string c_ExtraInfoName = "DecayHash"; /**< Name of the extraInfo, which is stored in each Particle **/
  };

} // Belle2 namespace

