/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao, Marko Staric                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FULLSIMMODULE_H_
#define FULLSIMMODULE_H_

#include <framework/core/Module.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <G4VisManager.hh>

#include <string>
#include <vector>

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

    MCParticleGraph m_mcParticleGraph;     /**< The MCParticle Graph used to manage the MCParticles before and after the simulation.*/

    G4VisManager* m_visManager;            /**< The parameter variable for manager display */

    std::string m_mcParticleInputColName;  /**< The parameter variable for the name of the input MCParticle collection. */
    std::string m_mcParticleOutputColName; /**< The parameter variable for the name of the output MCParticle collection. */
    double m_thresholdImportantEnergy;     /**< A particle which got 'stuck' and has less than this energy will be killed after m_thresholdTrials trials. */
    int m_thresholdTrials;                 /**< Geant4 will try m_thresholdTrials times to move a particle which got 'stuck' and has an energy less than m_thresholdImportantEnergy. */
    int m_trackingVerbosity;               /**< Tracking verbosity: 0=Silent; 1=Min info per step; 2=sec particles; 3=pre/post step info; 4=like 3 but more info; 5=proposed step length info. */
    int m_hadronprocessVerbosity;          /**< Hadron Process verbosity: 0=Silent; 1=info level; 2=debug level, default=0 */
    std::string m_physicsList;             /**< The name of the physics list which is used for the simulation. */
    bool m_optics;                         /*!< If set to true, registers the optical physics list. */
    double m_productionCut;                /*!< Apply continuous energy loss to primary particle which has no longer enough energy to produce secondaries which travel at least the specified productionCut distance. */
    int m_maxNumberSteps;                  /*!< The maximum number of steps before the track transportation is stopped and the track is killed. */
    double m_photonFraction;               /**< The fraction of Cerenkov photons which will be kept and propagated. */
    bool m_useNativeGeant4;                /**< If set to true, uses the Geant4 navigator and native detector construction class. */
    std::vector<std::string> m_uiCommands; /**< A list of Geant4 UI commands that should be applied before the simulation starts. */
    bool m_EnableVisualization;            /**< If set to true the Geant4 visualization support is enabled. */
    bool m_storeOpticalPhotons;            /**< controls storing of optical photons in MCParticles */
    bool m_storeSecondaries;               /**< contorls storing of Geant secondaries in MCParticles */
    double m_energyCut;                    /**< kinetic energy cut for the stored Geant secondaries */
    std::string m_magneticField;           /**< magnetic field stepper to use */
    double m_magneticCacheDistance;        /**< minimal distance for magnetic field lookup. If distance is smaller, return last value */
    double m_deltaChordInMagneticField;    /**< The maximum miss-distance between the trajectory curve and its linear chord(s) approximation */
    int m_trajectoryStore;                 /**< If true, store the trajectories of all primary particles */
    double m_trajectoryAngularTolerance;   /**< If >0, simplify the trajectory by merging almost parallel steps */
    double m_trajectoryDistanceTolerance;  /**< Maximum distance to actuall trajectory when merging points */

  private:

  };
}

#endif /* FULLSIMMODULE_H_ */
