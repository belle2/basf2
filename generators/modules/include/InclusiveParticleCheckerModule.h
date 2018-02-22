/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <boost/variant.hpp>
#include <set>

namespace Belle2 {
  /**
   * Check for the existence of an inclusive particle in the list of generated
   * particles.
   *
   * This module is intended for inclusive signal generation where we run a
   * generator which produces generic events and we require a certain particle
   * to be present in the resulting list of particles.
   */
  class InclusiveParticleCheckerModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of
     * the module.
     */
    InclusiveParticleCheckerModule();

    /** Register input and output data */
    virtual void initialize() override;
    /** Look for the particle and set the return type */
    virtual void event() override;
  private:
    /** Name or PDG string of the particle to look for */
    std::vector<boost::variant<std::string, int>> m_particles;
    /** If true require either the particle or its charge conjugate */
    bool m_includeConjugates{true};
    /** Maximum amount of tries per event to generate a event */
    unsigned int m_maxTries{10000};
    /** List of generated particles */
    StoreArray<MCParticle> m_mcParticles;
    /** Set of PDG codes to look for */
    std::set<int> m_particleCodes;
  };
}
