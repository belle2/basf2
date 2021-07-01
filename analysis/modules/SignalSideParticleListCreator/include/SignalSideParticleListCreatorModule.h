/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

namespace Belle2 {
  /**
   * The module creates a ParticleList and fills it with one of the daughter Particles.
   *
   * The module creates a ParticleList and fills it with one of the daughter Particles specified via the DecayString.
   * The module should be executed only in the RestOfEvent path (path for each ResoOfEvent object in the event) and
   * its main use case is in the construction of various vetos (e.g. pi0 veto, J/psi veto, ...), where one of the
   * daughter Particles is combined with any Particle found in the RestOfEvent to determine whether such pair
   * (not necessarily pair) is consistent with originating from the decay of some particle (pi0, J/psi, ...).
   *
   * Example: pi0 veto
   *
   * Signal side: B -> K* gamma
   * ParticleListName: 'gamma:sig'
   * DecayString: 'B -> K* ^gamma'
   *
   * As noted above the module has to be executed in ROE path, that is a path that is executed for each ROE object
   * found in the DataStore. The module first starts at the ROE object and obtains the related Particle. In our example
   * this is a B meson candidate. Next, the module fetches the selected B meson daughter (a photon in our case) and
   * adds it to the 'gamma:sig' ParticleList. This ParticleList contains only one particle, the photon used to reconstruct
   * the B -> K* gamma candidate.
   */
  class SignalSideParticleListCreatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SignalSideParticleListCreatorModule();

    /**  initialize method */
    virtual void initialize() override;

    /**  event method */
    virtual void event() override;


  private:

    std::string m_particleListName;  /**< Name of the ParticleList to be created and filled with signal side daughter Particle */
    std::string
    m_antiParticleListName;  /**< Name of the anti-ParticleList to be created and filled with signal side daughter Particle */
    std::string m_decayString;  /**< DecayString specifying the daughter Particle to be included in the ParticleList */

    int m_pdgCode = 0; /**< PDG code of the selected particle */

    DecayDescriptor m_pListDDescriptor;         /**< Decay descriptor of the particle list being selected */
    DecayDescriptor m_pDDescriptor;         /**< Decay descriptor of the particle being selected */

    bool m_isSelfConjugatedParticle =
      0;    /**< flag that indicates whether an anti-particle does not exist and therefore the output anti-ParticleList should not be created */

  };
}

