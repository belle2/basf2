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

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/dataobjects/MCInitialParticles.h>
#include <simulation/dataobjects/SimClockState.h>

#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/HardwareClockSettings.h>
#include <framework/dbobjects/BunchStructure.h>


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
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    // module steering parameters
    double m_coreGaussWidth; /**< sigma of core gaussian [ns] */
    double m_tailGaussWidth; /**< sigma of tail gaussian [ns] */
    double m_tailGaussFraction; /**< area fraction of core gaussian */
    double m_fixedT0 = NAN; /**< if set, a fixed t0 value is used instead of a gaussian distribution */
    double m_maximumT0 = NAN; /**< if set, randomize between -maximum and maximum */
    bool m_isCosmics = false; /**< if true L1 jitter for cosmics is simulated*/
    double m_coreGaussWidthCosmics; /**< sigma of core gaussian [ns] for cosmics*/
    double m_tailGaussWidthCosmics; /**< sigma of tail gaussian [ns] for cosmics*/
    double m_tailGaussFractionCosmics; /**< area fraction of core gaussian for cosmics*/

    // datastore collections
    StoreArray<MCParticle> m_mcParticles; /**< MC particles */
    StoreObjPtr<MCInitialParticles> m_initialParticles; /**< beam particles */
    StoreObjPtr<SimClockState> m_simClockState; /**< generated hardware clock state */

    // conditions DB objects
    DBObjPtr<HardwareClockSettings> m_clockSettings; /**< hardware clock settings */
    DBObjPtr<BunchStructure> m_bunchStructure; /**< bunch structure (fill pattern) */

  };

} // Belle2 namespace

