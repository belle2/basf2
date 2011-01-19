/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FULLSIMMODULE_H_
#define FULLSIMMODULE_H_

#include <framework/core/Module.h>

#include <string>

namespace Belle2 {

  /** The full Geant4 simulation module.
   *
   * This module provides the full Geant4 simulation for the framework.
   * It initializes Geant4, calls the converter to translate the ROOT TGeo
   * volumes/materials to native Geant4 volumes/materials, initializes the
   * physics processes and user actions.
   *
   * This module requires a valid geometry in memory (gGeoManager). Therefore,
   * a geometry building module should have been executed before this module is called.
   *
   */
  class FullSimModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    FullSimModule();

    /** Destructor of the module. */
    virtual ~FullSimModule();

    /**
     * Initialize the Module.
     *
     * Initializes Geant4, calls the geometry converter, creates the physics processes and
     * create the user actions.
     */
    virtual void initialize();

    /**
     * Called when a new run is started.
     *
     * Initializes the Geant4 run manager and sets the run number in Geant4.
     */
    virtual void beginRun();

    /**
     * Performs the full Geant4 simulation.
     */
    virtual void event();

    /**
     * Called when run has ended.
     */
    virtual void endRun();

    /**
     * Terminates the module.
     */
    virtual void terminate();


  protected:

    std::string m_mcParticleCollectionName; /**< The parameter variable for the name of the MCParticle collection. */
    double m_thresholdImportantEnergy;      /**< A particle which got 'stuck' and has less than this energy will be killed after m_thresholdTrials trials. */
    int m_thresholdTrials;                  /**< Geant4 will try m_thresholdTrials times to move a particle which got 'stuck' and has an energy less than m_thresholdImportantEnergy. */

  private:

  };
}

#endif /* FULLSIMMODULE_H_ */
