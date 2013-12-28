/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BBBREMINPUTMODULE_H
#define BBBREMINPUTMODULE_H

#include <framework/core/Module.h>
#include <generators/bbbrem/BBBrem.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <string>

namespace Belle2 {

  /**
   * The BBBrem Generator module.
   * Generates low scattering angle radiative Bhabha events (Beam-Beam Bremsstrahlung).
   */
  class BBBremInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    BBBremInputModule();

    /** Destructor. */
    virtual ~BBBremInputModule();

    /** Initializes the module. */
    virtual void initialize();

    /** Method is called for each event. */
    virtual void event();

    /** Method is called at the end of the event processing. */
    virtual void terminate();


  protected:

    /** Module parameters */
    int m_boostMode;      /**< The mode of the boost (0 = no boost, 1 = Belle II, 2 = Belle). */
    double m_photonEFrac; /**< Minimum photon energy fraction. */
    bool m_unweighted;    /**< True if BBBrem should produce unweighted events. */
    double m_maxWeight;   /**< The maximum weight. Used for the event rejection procedure to produce unweighted events. */

    /** Variables */
    BBBrem m_generator;        /**< The BBBrem generator. */
    MCParticleGraph m_mcGraph; /**< The MCParticle graph object. */

  };

} // end namespace Belle2


#endif /* BBBREMINPUTMODULE_H */
