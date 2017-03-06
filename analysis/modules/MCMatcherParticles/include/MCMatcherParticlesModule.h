/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCMATCHERPARTICLESMODULE_H
#define MCMATCHERPARTICLESMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <analysis/dataobjects/ParticleList.h>
#include <string>


namespace Belle2 {

  /**
   * MC matching module: module performs MC matching (sets the relation Particle -> MCParticle) for all particles
   * and its (grand)^N-daughter particles in the user-specified ParticleList. The MC matching algorithm is implemented
   * in the MCMatching namespace.
   *
   * In addition to the usual mc matching agorithm the module can run also loose mc matching. The difference between
   * loose and normal mc matching algorithm is that the loose agorithm will find the common mother of the majority of
   * daughter particles while the normal algorithm finds the common mother of all daughters. In the case one of the
   * daughters has no generated mother (or mc association) the normal mc matching algorithm will tag such candidate
   * as background (no mc match), while the loose algorithm will find the common mother of remaining daughters. This
   * is usefull for identifying for example self cross-feed candidates, where usually one of the photons is wrong,
   * but the rest of the decay cahin is correctly reconstructed. The results of loose mc matching algorithm are
   * stored to the following extraInfo items:
   * - looseMCMotherPDG: PDG code of most common mother
   * - looseMCMotherIndex: 1-based StoreArray<MCParticle> index of most common mother
   * - looseMCWrongDaughterN: number of daughters that don't originate from the most
   *                          common mother
   * - looseMCWrongDaughterPDG: PDG code of the daughter that doesn't orginate from
   *                            the most common mother
   *                            (only if looseMCWrongDaughterN = 1)
   * - looseMCWrongDaughterBiB: 1 if the wrong daughter is Beam Induced Background
   *                            Particle
   */
  class MCMatcherParticlesModule : public Module {

  public:

    /**
     * Constructor
     */
    MCMatcherParticlesModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    std::string m_listName;  /**< steering variable: name of the input ParticleList */
    StoreObjPtr<ParticleList> m_plist; /**< the input ParticleList. */

    bool m_looseMatching; /**< perform loose mc matching */

    /**
     * Finds common mother of the majority of daughters. The results are stored to extraInfo.
     */
    void setLooseMCMatch(const Particle* particle);
  };
}
#endif
