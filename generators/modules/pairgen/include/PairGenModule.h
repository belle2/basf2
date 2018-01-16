/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>

namespace Belle2 {

  /** The PairGen module.
   * Simplistic generator of track pairs back to back in CMS.
   * Stores them into the MCParticle class.
   */
  class PairGenModule : public Module {
  public:
    /**
     * Constructor.
     * Sets the module parameters.
     */
    PairGenModule();

    /** Destructor. */
    virtual ~PairGenModule() {}

    /** Initializes the module. */
    void initialize();

    /** Method is called for each event. */
    void event();

  protected:
    int m_PDG; /** Particles PDG code */
    bool m_saveBoth; /** Store both particles if true, one if false */
    //TODO other parameters to make the module more useful

    MCParticleGraph m_particleGraph;
  };

} // end namespace Belle2
