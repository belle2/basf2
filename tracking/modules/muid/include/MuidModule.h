/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MUIDMODULE_H
#define MUIDMODULE_H

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>

#include <string>
#include <vector>

namespace Belle2 {

  class TrackExtrapolateG4e;

  /** The geant4e-based muon identification module.
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

    //! Name of the Track collection of the reconstructed tracks to be extrapolated
    std::string m_TracksColName;

    //! Name of the RecoTrack collection of the reconstructed tracks to be extrapolated
    std::string m_RecoTracksColName;

    //! Name of the extHit collection of the extrapolation hits
    std::string m_ExtHitsColName;

    //! Name of the muid collection of the muon identification information
    std::string m_MuidsColName;

    //! Name of the muidHit collection of the extrapolation hits
    std::string m_MuidHitsColName;

    //! Name of the BKLM 2D hits collection
    std::string m_BKLMHitsColName;

    //! Name of the EKLM 2D hits collection
    std::string m_EKLMHitsColName;

    //! Name of the KLMCluster collection
    std::string m_KLMClustersColName;

    //! Name of the ECLCluster collection
    std::string m_ECLClustersColName;

    //! Name of the TrackClusterSeparation collection
    std::string m_TrackClusterSeparationsColName;

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

    //! user-defined maximum squared-distance (#variances) for matching hit to extrapolation
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

    //! A list of Geant4 UI commands that should be applied before the extrapolation starts
    std::vector<std::string> m_UICommands;

  private:

    //! Pointer to the TrackExtrapoleG4e singleton
    TrackExtrapolateG4e* m_Extrapolator;

  };

} // end of namespace Belle2

#endif // MUIDMODULE_H
