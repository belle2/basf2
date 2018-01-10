/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <cmath>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/dataobjects/MCInitialParticles.h>


namespace Belle2 {

  /**
   * Module generates discrete event t0 in ~4ns steps (bunch spacing) according to
   * (double) gaussian distribution and adds it to the production and decay times of
   * MCParticles. This means that after this module the time origin (t = 0) is set
   * according to what L1 trigger thinks is the collision time.
   */
  class EventT0GeneratorModule : public Module {

  public:

    /**
     * Constructor
     */
    EventT0GeneratorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Event processor.
     */
    virtual void event();

  private:

    // module steering parameters
    double m_coreGaussWidth; /**< sigma of core gaussian [ns] */
    double m_tailGaussWidth; /**< sigma of tail gaussian [ns] */
    double m_tailGaussFraction; /**< area fraction of core gaussian */
    double m_fixedT0 = NAN; /**< if set, a fixed t0 value is used instead of a gaussian distrubtion */

    // other
    double m_bunchTimeSep = 0;         /**< time between two bunches */
    StoreArray<MCParticle> m_mcParticles; /**< MC particles */
    StoreObjPtr<MCInitialParticles> m_initialParticles; /**< beam particles */

  };

} // Belle2 namespace

