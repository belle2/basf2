/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// dataobjects
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <string>


namespace Belle2 {

  /**
   * MC matching module: module performs MC matching (sets the relation Particle -> MCParticle) for all particles
   * and its (grand)^N-daughter particles in the user-specified ParticleList. The MC matching algorithm is implemented
   * in the MCMatching namespace.
   *
   * In addition to the usual mc matching algorithm the module can run also loose mc matching. The difference between
   * loose and normal mc matching algorithm is that the loose algorithm will find the common mother of the majority of
   * daughter particles while the normal algorithm finds the common mother of all daughters. In the case one of the
   * daughters has no generated mother (or mc association) the normal mc matching algorithm will tag such candidate
   * as background (no mc match), while the loose algorithm will find the common mother of remaining daughters. This
   * is useful for identifying for example self cross-feed candidates, where usually one of the photons is wrong,
   * but the rest of the decay chain is correctly reconstructed. The results of loose mc matching algorithm are
   * stored to the following extraInfo items:
   * - looseMCMotherPDG: PDG code of most common mother
   * - looseMCMotherIndex: 1-based StoreArray<MCParticle> index of most common mother
   * - looseMCWrongDaughterN: number of daughters that don't originate from the most
   *                          common mother
   * - looseMCWrongDaughterPDG: PDG code of the daughter that doesn't originate from
   *                            the most common mother
   *                            (only if looseMCWrongDaughterN = 1)
   * - looseMCWrongDaughterBiB: 1 if the wrong daughter is Beam Induced Background
   *                            Particle
   *
   * In addition can perform tag matching for (ccbar) tags, requires that normal MC matching has already
   * been performed and set relations. Low energy photons with energy < 0.1 GeV and ISR are ignored.
   * The results of (ccbar) tag matching algorithm are stored to the following extraInfo items:
   * - ccbarTagSignal: 1st digit is status of signal particle, 2nd digit is Nleft-1, 3rd digit is NextraFSP.
   * - ccbarTagMCpdg: PDG code of (charm) hadron outside tag (signal side).
   * - ccbarTagMCpdgMother: PDG code of the mother of the (charm) hadron outside tag (signal side).
   * - ccbarTagNleft: number of particles (composites have priority) left outisde tag.
   * - ccbarTagNextraFSP: number of extra FSP particles attached to the tag.
   * - ccbarTagSignalStatus: status of the targeted signal side particle.
   * - ccbarTagNwoMC: number of daughters without MC match.
   * - ccbarTagNwoMCMother: number of daughters without MC mother.
   * - ccbarTagNnoAllMother: number of daughters without common allmother.
   * - ccbarTagNmissGamma: number of daughters with missing gamma mc error.
   * - ccbarTagNmissNeutrino: number of daughters with missing neutrino mc error.
   * - ccbarTagNdecayInFlight: number of daughters with decay in flight mc error.
   * - ccbarTagNsevereMCError: number of daughters with severe mc error.
   * - ccbarTagNmissRecoDaughters: number of daughters with any mc error.
   * - ccbarTagNleft2ndPDG: PDG of one particle left additionally to the signal particle.
   * - ccbarTagAllMotherPDG: PDG code of the allmother (Z0 or virtual photon).
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
    StoreArray<MCParticle> m_mcparticles; /**< the array of MCParticles. */
    StoreArray<Particle> m_particles; /**< the array of Particles. */

    bool m_looseMatching; /**< perform loose mc matching */
    bool m_ccbarTagMatching; /**< perform ccbar tag matching */

    /**
     * Finds common mother of the majority of daughters. The results are stored to extraInfo.
     */
    void setLooseMCMatch(const Particle* particle);


    /**
     * returns 1 if the eventParticle daughters were all caught in recParticles, 2 if partially and 0 if none.
     * it is used to check if a signal particle is still left on the signal side of the event (what is not caught by the tag side).
     */
    int ccbarTagPartialHelper(
      const MCParticle* mcParticle,
      std::vector<const Particle*>& fspParticles,
      std::vector<const MCParticle*>& missedParticles
    );

    /**
     * returns 1 if the mcParticle daughters were all caught in tag, 2 if partially and 0 if none.
     * it is used to check what if the signal side particle has been particle absorbed in the tag.
     */
    int ccbarTagPartialHelper(
      const MCParticle* mcParticle,
      const std::vector<const Particle*>& fspParticles
    );

    /**
     * Investigates the composition of the tag and remaining signal side and saves the inforamtion to extraInfo.
     */
    void setCCbarTagMatch(const Particle* particle);
  };
}
