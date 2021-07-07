/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef FULLSIMMODULE_H_
#define FULLSIMMODULE_H_

#include <framework/core/Module.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <string>
#include <vector>

class G4MagneticField;
class G4Mag_UsualEqRhs;
class G4MagIntegratorStepper;
class G4ChordFinder;
class G4VisManager;
class G4StepLimiter;

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
    virtual void initialize() override;

    /**
     * Called when a new run is started.
     *
     * Initializes the Geant4 run manager and sets the run number in Geant4.
     */
    virtual void beginRun() override;

    /**
     * Performs the full Geant4 simulation.
     */
    virtual void event() override;

    /**
     * Called when run has ended.
     */
    virtual void endRun() override;

    /**
     * Terminates the module.
     */
    virtual void terminate() override;


  protected:

    std::string m_mcParticleInputColName;  /**< The parameter variable for the name of the input MCParticle collection. */
    double m_thresholdImportantEnergy;     /**< A particle which got 'stuck' and has less than this energy will be killed after m_thresholdTrials trials. */
    int m_thresholdTrials;                 /**< Geant4 will try m_thresholdTrials times to move a particle which got 'stuck' and has an energy less than m_thresholdImportantEnergy. */
    int m_runEventVerbosity;               /**< Geant4 run/event verbosity: 0=Silent; 1=info level; 2=debug level, default=0 */
    int m_trackingVerbosity;               /**< Tracking verbosity: 0=Silent; 1=Min info per step; 2=sec particles; 3=pre/post step info; 4=like 3 but more info; 5=proposed step length info. */
    int m_hadronProcessVerbosity;          /**< Hadron Process verbosity: 0=Silent; 1=info level; 2=debug level, default=0 */
    int m_emProcessVerbosity;              /**< Loss Table verbosity: 0=Silent; 1=info level; 2=debug level, default=0 */
    std::string m_physicsList;             /**< The name of the physics list which is used for the simulation. */
    bool m_standardEM;                     /*!< If set to true, replaces fast EM physics with standard EM physics. */
    bool m_optics;                         /*!< If set to true, registers the optical physics list. */
    bool m_HPneutrons;                     /*!< If true, high precision neutron models used below 20 MeV. */
    bool m_monopoles;                      /*!< If set to true, G4MonopolePhysics is registered in Geant4 PhysicsList.*/
    double m_monopoleMagneticCharge;       /*!< The value of monopole magnetic charge in units of e+.*/
    double m_productionCut;                /*!< Apply continuous energy loss to primary particle which has no longer enough energy to produce secondaries which travel at least the specified productionCut distance. */
    double m_pxdProductionCut;             /*!< Secondary production threshold in PXD envelope. */
    double m_svdProductionCut;             /*!< Secondary production threshold in SVD envelope. */
    double m_cdcProductionCut;             /*!< Secondary production threshold in CDC envelope. */
    double m_arichtopProductionCut;        /*!< Secondary production threshold in ARICH and TOP envelopes. */
    double m_eclProductionCut;             /*!< Secondary production threshold in ECL envelopes. */
    double m_klmProductionCut;             /*!< Secondary production threshold in BKLM and EKLM envelopes. */
    int m_maxNumberSteps;                  /*!< The maximum number of steps before the track transportation is stopped and the track is killed. */
    double m_photonFraction;               /**< The fraction of Cerenkov photons which will be kept and propagated. */
    bool m_useNativeGeant4;                /**< If set to true, uses the Geant4 navigator and native detector construction class. */
    std::vector<std::string> m_uiCommandsAtPreInit; /**< A list of Geant4 UI commands that should be applied at PreInit state,
                                                         before the Geant4 initialization and before the simulation starts. */
    std::vector<std::string> m_uiCommandsAtIdle;    /**< A list of Geant4 UI commands that should be applied at Idle state,
                                                         after the Geant4 initialization and before the simulation starts. */
    bool m_EnableVisualization;            /**< If set to true the Geant4 visualization support is enabled. */

    bool m_storeOpticalPhotons;            /**< controls storing of optical photons in MCParticles */
    bool m_storeSecondaries;               /**< controls storing of Geant secondaries in MCParticles */
    double m_secondariesEnergyCut;         /**< kinetic energy cut for the stored Geant secondaries */
    bool m_storeBremsstrahlungPhotons;     /**< controls storing of bremsstrahlung photons in MCParticles */
    double m_bremsstrahlungPhotonsEnergyCut;/**< kinetic energy cut for the stored bremsstrahlung photons */
    bool m_storePairConversions;           /**< controls storing of e+ or e- from pair conversions in MCParticles */
    double m_pairConversionsEnergyCut;     /**< kinetic energy cut for the stored e+ or e- from pair conversions */

    std::string m_magneticFieldName;       /**< magnetic field stepper to use */
    double m_magneticCacheDistance;        /**< minimal distance for magnetic field lookup. If distance is smaller, return last value */
    double m_deltaChordInMagneticField;    /**< The maximum miss-distance between the trajectory curve and its linear chord(s) approximation */

    int m_trajectoryStore;                 /**< If true, store the trajectories of all primary particles */
    double m_trajectoryDistanceTolerance;  /**< Maximum distance to actuall trajectory when merging points */
    std::vector<float> m_absorbers;        /**< The absorbers defined at given radii where tracks across them will be destroyed.
                                                This set is used in the PXD only simulation for PXD gain calibration.*/



  private:

    /** The MCParticle Graph used to manage the MCParticles before and after the simulation.*/
    MCParticleGraph m_mcParticleGraph;

    /** Pointer to the uncached magnetic field (might be superseded by its cached version) */
    G4MagneticField* m_uncachedField;

    /** Pointer to the (un)cached magnetic field */
    G4MagneticField* m_magneticField;

    /** Pointer to the equation of motion in the magnetic field (if not the default) */
    G4Mag_UsualEqRhs* m_magFldEquation;

    /** Pointer to the equation-of-motion stepper (if not the default) */
    G4MagIntegratorStepper* m_stepper;

    /** Pointer to the equation-of-motion chord finder (if not the default) */
    G4ChordFinder* m_chordFinder;

    /** Pointer to the visualization manager (if used) */
    G4VisManager* m_visManager;

    /** Pointer to the step limiter */
    G4StepLimiter* m_stepLimiter;

  };
}

#endif /* FULLSIMMODULE_H_ */
