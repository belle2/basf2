/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PAIRGENMODULE_H
#define PAIRGENMODULE_H

#include <framework/core/Module.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <TRandom3.h>
#include <TLorentzVector.h>

namespace Belle2 {

  /** The PairGen module.
   * Generate pair of tracks back to back in CMS and store them
   * into the MCParticle class.
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
    /** Particles PDG code */
    int m_PDG;

    MCParticleGraph m_particleGraph;
  };

} // end namespace Belle2

#endif // PAIRGENMODULE_H

