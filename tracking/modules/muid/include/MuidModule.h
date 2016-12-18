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
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/ExtHit.h>
#include <framework/gearbox/Const.h>
#include <simulation/kernel/ExtManager.h>
#include <simulation/kernel/ExtCylSurfaceTarget.h>
#include <bklm/geometry/GeometryPar.h>
#include <G4ThreeVector.hh>
#include <G4Point3D.hh>
#include <G4Vector3D.hh>
#include <G4ErrorTrajErr.hh>
#include <TVector3.h>
#include <TMatrixDSym.h>

class G4RunManager;
class G4VUserPhysicsList;
class G4UserTrackingAction;
class G4UserSteppingAction;
class G4VPhysicalVolume;
class G4ErrorFreeTrajState;
class G4String;
namespace genfit { class AbsTrackRep; }

namespace Belle2 {

  class Muid;
  class MuidHit;
  class MuidPar;
  class Track;
  class RecoTrack;

  //! possible intersection of extrapolated track with a KLM layer
  struct Point {
    //! flag to indicate if this point is in the barrel (true) or endcap (false)
    bool        inBarrel;
    //! flag to indicate if this point is in the forward (true) or backward (false) end
    bool        isForward;
    //! sector number (0..7 for barrel, 0..3 for endcap) of this point
    int         sector;
    //! layer number (0..14 for barrel, 0..13 for endcap) of this point
    int         layer;
    //! extrapolated-track global position (cm) of this intersection
    TVector3    position;
    //! extrapolated-track momentum (GeV/c) at this intersection
    TVector3    momentum;
    //! extrapolated-track phase-space covariance matrix at this intersection
    TMatrixDSym covariance;
    //! extrapolated-track position (cm) projected to the 2D hit's midplane
    TVector3    positionAtHitPlane;
    //! time (ns) of matching BKLMHit2d
    double      time;
    //! chi-squared value of transverse deviation between extrapolated and measured hit positions
    double      chi2;
  };

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
    virtual ~MuidModule();

    //! Initialize for muon identification
    virtual void initialize();

    //! Perform beginning-of-run actions
    virtual void beginRun();

    //! Performs muon identification for all tracks in one event
    virtual void event();

    //! Perform end-of-run actions
    virtual void endRun();

    //! Terminates the module
    virtual void terminate();

  protected:

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

    //! Flag for source (0 = beam, 1 = cosmic ray)
    int m_Cosmic;

    //! Name of the Track collection of the reconstructed tracks to be extrapolated
    std::string m_TracksColName;

    //! Name of the muid collection of the muon identification information
    std::string m_MuidsColName;

    //! Name of the muidHit collection of the extrapolation hits
    std::string m_MuidHitsColName;

    //! Name of the extHit collection of the extrapolation hits
    std::string m_ExtHitsColName;

    //! Name of the KLMCluster collection
    std::string m_KLMClustersColName;

    //! Name of the TrackClusterSeparation collection
    std::string m_TrackClusterSeparationsColName;

    //! Name of the BKLM 2D hits collection
    std::string m_BKLMHitsColName;

    //! Name of the EKLM 2D hits collection
    std::string m_EKLMHitsColName;

    //! Tracking verbosity: 0=Silent; 1=Min info per step; 2=sec particles; 3=pre/post step info; 4=like 3 but more info; 5=proposed step length info
    int m_trackingVerbosity;

    //! A list of Geant4 UI commands that should be applied before the extrapolation starts
    std::vector<std::string> m_uiCommands;

    //! If set to true the Geant4 visualization support is enabled
    bool m_enableVisualization;

    //! magnetic field stepper to use
    std::string m_magneticFieldName;

    //! minimal distance for magnetic field lookup. If distance is smaller, return cached value
    double m_magneticCacheDistance;

    //! maximum miss-distance between the trajectory curve and its linear chord(s) approximation
    double m_deltaChordInMagneticField;

  private:

    //! Register the list of geant4 physical volumes whose entry/exit
    //! points will be saved during extrapolation
    void registerVolumes();

    //! Get the physical volume information for a geant4 physical volume
    void getVolumeID(const G4TouchableHandle&, Const::EDetector&, int&);

    //! Get the starting phase-space point and covariance for one reconstructed track and PDG hypothesis
    void getStartPoint(RecoTrack* recoTrack, const genfit::AbsTrackRep* gfTrackRep, int pdgCode, G4ThreeVector&, G4ThreeVector&,
                       G4Point3D&, G4Vector3D&, G4ErrorTrajErr&);

    //! Add an extrapolation point for the track
    bool createHit(G4ErrorFreeTrajState*, Track&, int);

    //! Add a KLM sensitive-volume entry/exit extHit for the track
    void createEntryExitHit(const G4ErrorFreeTrajState*, ExtHitStatus, Track&, int);

    //! Find the intersection point of the track with the crossed BKLM plane
    bool findBarrelIntersection(const TVector3&, Point&);

    //! Find the intersection point of the track with the crossed EKLM plane
    bool findEndcapIntersection(const TVector3&, Point&);

    //! Find the matching BKLM 2D hit nearest the intersection point of the track with the crossed BKLM plane
    bool findMatchingBarrelHit(Point&, const Track&);

    //! Find the matching EKLM 2D hit nearest the intersection point of the track with the crossed EKLM plane
    bool findMatchingEndcapHit(Point&, const Track&);

    //! Nudge the track using the matching hit
    void adjustIntersection(Point&, const double*, const TVector3&, const TVector3&);

    //! Complete muon identification after end of track extrapolation
    void finishTrack(Muid*, int);

    //! Convert GEANT4e covariance to phase-space covariance
    void fromG4eToPhasespace(const G4ErrorFreeTrajState*, TMatrixDSym&);

    //! Convert phase-space covariance to GEANT4e covariance
    void fromPhasespaceToG4e(const TVector3&, const TMatrixDSym&, G4ErrorTrajErr&);

    //! Pointer to the ExtManager singleton
    Simulation::ExtManager* m_ExtMgr;

    //! PDG code for the particle-ID hypotheses
    std::vector<int> m_PDGCode;

    //!  ChargedStable hypotheses
    std::vector<Const::ChargedStable> m_ChargedStable;

    //! Pointers to BKLM geant4 sensitive (physical) volumes
    std::vector<G4VPhysicalVolume*>* m_BKLMVolumes;

    //! Pointers to EKLM geant4 sensitive (physical) volumes
    std::vector<G4VPhysicalVolume*>* m_EKLMVolumes;

    //! Pointers to geant4 physical volumes whose entry/exit points will be saved
    std::vector<G4VPhysicalVolume*>* m_EnterExit;

    //! Time of flight (ns) along the track from the interaction point
    double m_TOF;

    //! Minimum squared radius (cm) outside of which extrapolation will continue
    double m_MinRadiusSq;

    //! virtual "target" cylinder (boundary beyond which extrapolation ends)
    Simulation::ExtCylSurfaceTarget* m_Target;

    //! offset (cm) along z axis of KLM midpoint from IP
    double m_OffsetZ;

    //! maximum radius (cm) of the endcaps
    double m_EndcapMaxR;

    //! minimum radius (cm) of the endcaps
    double m_EndcapMinR;

    //! minimum radius (cm) of the barrel
    double m_BarrelMinR;

    //! maximum radius (cm) of the barrel
    double m_BarrelMaxR;

    //! half-length (cm) of either endcap
    double m_EndcapHalfLength;

    //! half-length (cm) of the barrel
    double m_BarrelHalfLength;

    //! outermost forward-endcap layer that is active for muon identification (user-defined)
    int m_OutermostActiveForwardEndcapLayer;

    //! outermost backward-endcap layer that is active for muon identification (user-defined)
    int m_OutermostActiveBackwardEndcapLayer;

    //! outermost barrel layer that is active for muon identification (user-defined)
    int m_OutermostActiveBarrelLayer;

    //! midpoint along z (cm) of the forward endcap from the KLM midpoint
    double m_EndcapMiddleZ;

    //! user-defined maximum squared-distance (#variances) for matching hit to extrapolation
    double m_MaxDistSqInVariances;

    //! user-defined maximum cone angle (radians) between KLMCluster and associated track
    double m_MaxClusterTrackConeAngle;

    //! BKLM RPC phi-measuring strip position variance (cm^2) by layer
    double m_BarrelPhiStripVariance[NLAYER + 1];

    //! BKLM RPC z-measuring strip position variance (cm^2) by layer
    double m_BarrelZStripVariance[NLAYER + 1];

    //! BKLM scintillator strip position variance (cm^2)
    double m_BarrelScintVariance;

    //! EKLM scintillator strip position variance (cm^2)
    double m_EndcapScintVariance;

    //! hit-plane radius (cm) at closest distance to IP of each barrel end | sector | layer
    double m_BarrelModuleMiddleRadius[2][NSECTOR + 1][NLAYER + 1];

    //! hit-plane z (cm) of each IP layer relative to KLM midpoint
    double m_EndcapModuleMiddleZ[NLAYER + 1];

    //! normal unit vector of each barrel sector
    TVector3 m_BarrelSectorPerp[NSECTOR + 1];

    //! azimuthal unit vector of each barrel sector
    TVector3 m_BarrelSectorPhi[NSECTOR + 1];

    //! flag to indicate whether the track is in the forward (true) or backward (false) region (for endcaps asymmetry)
    bool m_IsForward;

    //! outermost barrel layer encountered by the extrapolated track in the prior steps
    int m_FirstBarrelLayer;

    //! outermost barrel layer encountered by the extrapolated track in the prior steps
    int m_FirstEndcapLayer;

    //! accumulated bit pattern of layers crossed by the extrapolated track
    int m_ExtLayerPattern;

    //! accumulated bit pattern of layers with matching hits
    int m_HitLayerPattern;

    //! flag to indicate that the extrapolated track escaped from the KLM
    bool m_Escaped;

    //! accumulated chi-squared of all in-plane transverse deviations between extrapolation and matching hit
    double m_Chi2;

    //! accumulated number of points with matching 2D hits
    int m_NPoint;

    //! outermost barrel layer crossed by the extrapolated track
    int m_LastBarrelExtLayer;

    //! outermost barrel layer with a matching hit
    int m_LastBarrelHitLayer;

    //! outermost endcap layer crossed by the extrapolated track
    int m_LastEndcapExtLayer;

    //! outermost endcap layer with a matching hit
    int m_LastEndcapHitLayer;

    //! probability density function for positive-muon hypothesis
    MuidPar* m_MuonPlusPar;

    //! probability density function for negative-muon hypothesis
    MuidPar* m_MuonMinusPar;

    //! probability density function for positive-pion hypothesis
    MuidPar* m_PionPlusPar;

    //! probability density function for negative-pion hypothesis
    MuidPar* m_PionMinusPar;

    //! probability density function for positive-kaon hypothesis
    MuidPar* m_KaonPlusPar;

    //! probability density function for negative-kaon hypothesis
    MuidPar* m_KaonMinusPar;

    //! probability density function for proton hypothesis
    MuidPar* m_ProtonPar;

    //! probability density function for antiproton hypothesis
    MuidPar* m_AntiprotonPar;

    //! probability density function for deuteron hypothesis
    MuidPar* m_DeuteronPar;

    //! probability density function for antideuteron hypothesis
    MuidPar* m_AntideuteronPar;

    //! probability density function for electron hypothesis
    MuidPar* m_ElectronPar;

    //! probability density function for positron hypothesis
    MuidPar* m_PositronPar;

  };

} // end of namespace Belle2

#endif // MUIDMODULE_H
