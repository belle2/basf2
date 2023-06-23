/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>

/* C++ headers. */
#include <string>
#include <vector>

namespace Belle2 {

  class TrackExtrapolateG4e;

  /**
   * The geant4e-based muon identification module.
   *
   * This module extrapolates tracks outward from the outer perimeter of the CDC
   * using geant4e through the ECL and KLM and uses the matching hits to
   * determine whether each track is muon-like or not.
   *
   * This module requires a valid geometry in memory (gGeoManager). Therefore,
   * a geometry building module should have been executed before this module is called.
   *
   */
  class MuidModule : public Module {

  public:

    //! constructor
    MuidModule();

    //! destructor
    ~MuidModule();

    //! Initialize for muon identification
    void initialize() override;

    //! Perform beginning-of-run actions
    void beginRun() override;

    //! Performs muon identification for all tracks in one event
    void event() override;

    //! Perform end-of-run actions
    void endRun() override;

    //! Terminates the module
    void terminate() override;

  protected:

    //! PDG code for the particle-ID hypotheses
    std::vector<int> m_PDGCodes;

    //!  ChargedStable hypotheses
    std::vector<Const::ChargedStable> m_Hypotheses;

    //! Mean hit - trigger time (ns)
    double m_MeanDt;

    //! Coincidence window half-width for in-time KLM hits (ns)
    double m_MaxDt;

    //! Minimum transverse momentum in GeV/c for extrapolation to be started
    double m_MinPt;

    //! Minimum kinetic energy in GeV for extrapolation to continue
    double m_MinKE;

    //! Maximum step size in cm (0 for no upper limit)
    double m_MaxStep;

    //! user-defined maximum squared-distance (in number of variances) for matching hit to extrapolation
    double m_MaxDistSqInVariances;

    //! user-defined maximum distance in cm between KLMCluster and associated track
    double m_MaxKLMTrackClusterDistance;

    //! user-defined maximum distance in cm between ECLCluster and associated track
    double m_MaxECLTrackClusterDistance;

    //! Tracking verbosity: 0=Silent; 1=Min info per step; 2=sec particles; 3=pre/post step info; 4=like 3 but more info; 5=proposed step length info
    int m_TrackingVerbosity;

    //! If set to true the Geant4 visualization support is enabled
    bool m_EnableVisualization;

    //! magnetic field stepper to use
    std::string m_MagneticFieldStepperName;

    //! minimal distance for magnetic field lookup. If distance is smaller, return cached value
    double m_MagneticCacheDistance;

    //! maximum miss-distance between the trajectory curve and its linear chord(s) approximation
    double m_DeltaChordInMagneticField;

    //! Parameter to add the found hits also to the reco tracks or not. Is turned off by default.
    bool m_addHitsToRecoTrack = false;

    //! A list of Geant4 UI commands that should be applied before the extrapolation starts
    std::vector<std::string> m_UICommands;

  private:

    //! Pointer to the TrackExtrapoleG4e singleton
    TrackExtrapolateG4e* m_Extrapolator;

  };

} // end of namespace Belle2
