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
#include <framework/datastore/RelationArray.h>
#include <tracking/dataobjects/ExtHit.h>
#include <framework/gearbox/Const.h>
#include <simulation/kernel/ExtManager.h>
#include <simulation/kernel/ExtCylSurfaceTarget.h>
#include <bklm/geometry/GeometryPar.h>
#include <CLHEP/Vector/ThreeVector.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <TVector3.h>

#include <G4TouchableHandle.hh>
#include <G4ErrorTrajErr.hh>
#include <TMatrixD.h>

class G4RunManager;
class G4VUserPhysicsList;
class G4UserTrackingAction;
class G4UserSteppingAction;
class G4VPhysicalVolume;
class G4ErrorFreeTrajState;
class G4String;
namespace genfit { class Track; }

#define BARREL 1
#define ENDCAP 2

#define FORWARD 1
#define BACKWARD 2

#define MISSED 1
#define CROSSED 2
#define SIDE 3

namespace Belle2 {

  class Muid;
  class MuidHit;
  class BKLMHit2d;
  class EKLMHit2d;
  class MuidPar;

  //! detector-element identifier
  struct Address {
    //! flag to indicate if this element is in an RPC (true) or scintillator (false)
    bool isRPC;
    //! flag to indicate if this element is in the barrel (true) or endcap (false)
    bool inBarrel;
    //! flag to indicate if this element is in the forward (true) or backward (false) end
    bool isForward;
    //! flag to indicate if this element is in the inner (true) or outer (false) detection plane
    bool isInnerPlane;
    //! sector number (0..7 for barrel, 0..3 for endcap) of this element
    int  sector;
    //! layer number (0..14 for barrel, 0..13 for endcap) of this element
    int  layer;
  };

  //! possible intersection of extrapolated track with a KLM layer
  struct Point {
    //! flag to indicate if this intersection is in a sensitive volume
    bool                enteredSensitiveVolume;
    //!  state of the intersection
    int                 intersected;
    //! flag to indicate if the intersection has a matching 2D detector hit
    bool                hasMatchingHit;
    //! detector-element identifier of this intersection
    Address             address;
    //! extrapolated-track global position (cm) of this intersection
    TVector3            position;
    //! extrapolated-track momentum (GeV/c) at this intersection
    TVector3            momentum;
    //! extrapolated-track phase-space covariance matrix at this intersection
    CLHEP::HepSymMatrix covariance;
    //! extrapolated-track position (cm) projected to the hit plane
    TVector3            positionAtHitPlane;
    //! chi-squared value of transverse deviation between extrapolated and measured hit positions
    double              chi2;
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

    //! Minimum transverse momentum in GeV/c for extrapolation to be started
    double m_minPt;

    //! Minimum kinetic energy in GeV for extrapolation to continue
    double m_minKE;

    //! Maximum step size in cm (0 for no upper limit)
    double m_maxStep;

    //! Flag for source (0 = beam, 1 = cosmic ray)
    int    m_cosmic;

    //! Name of the Track collection of the reconstructed tracks to be extrapolated
    std::string m_tracksColName;

    //! Name of the muidLikelihood collection of the muon identification information
    std::string m_muidLikelihoodsColName;

    //! Name of the muid collection of the muon identification information
    std::string m_muidsColName;

    //! Name of the muidHit collection of the extrapolation hits
    std::string m_muidHitsColName;

    //! Name of the BKLM 2D hits collection
    std::string m_bklmHitsColName;

    //! Name of the EKLM 2D hits collection
    std::string m_eklmHitsColName;

  private:

    //! Register the list of geant4 physical volumes whose entry/exit
    //! points will be saved during extrapolation
    void registerVolumes();

    //! Get the physical volume information for a geant4 physical volume
    void getVolumeID(const G4TouchableHandle&, ExtDetectorID&, int&);

    //! Convert the geant4e covariance to phasespacePoint 6x6 covariance
    TMatrixD getCov(const G4ErrorFreeTrajState*);

    //! Get the starting phase-space point and covariance for one reconstructed track and PDG hypothesis
    void getStartPoint(const genfit::Track*, int, G4ThreeVector&, G4ThreeVector&, G4ErrorTrajErr&);

    //! Add an extrapolation point for the track
    bool createHit(G4ErrorFreeTrajState*, int, int, StoreArray<MuidHit>&, RelationArray&, const StoreArray<BKLMHit2d>&, const StoreArray<EKLMHit2d>&);

    //! Find the intersection point of the track with the crossed BKLM plane
    void findBarrelIntersection(Point&, const TVector3&, const TVector3&);

    //! Find the intersection point of the track with the crossed EKLM plane
    void findEndcapIntersection(Point&, const TVector3&, const TVector3&);

    //! Find the matching BKLM 2D hit nearest the intersection point
    //! of the track with the crossed BKLM plane
    void findMatchingBarrelHit(Point&, const StoreArray<BKLMHit2d>&);

    //! Find the matching EKLM 2D hit nearest the intersection point
    //! of the track with the crossed EKLM plane
    void findMatchingEndcapHit(Point&, const StoreArray<EKLMHit2d>&);

    //! Nudge the track using the matching hit
    void adjustIntersection(Point&, const double*, const TVector3&);

    //! Get the in-plane covariance
    double getPlaneVariance(const CLHEP::HepSymMatrix&, const TVector3&, const TVector3&);

    //! Complete muon identification after end of track extrapolation
    void finishTrack(Muid*);

    //! Convert GEANT4 physical volume in BKLM/EKLM to an Address
    void getAddress(int, int, Address&);

    //! Convert GEANT4e covariance to phase-space covariance
    void fromG4eToPhasespace(const G4ErrorFreeTrajState*, CLHEP::HepSymMatrix&);

    //! Convert phase-space covariance to GEANT4e covariance
    void fromPhasespaceToG4e(const Point&, G4ErrorTrajErr&);

    //! Fill PDF tables
    void fillPDF(int);

    //! Calculate spline-interpolation coefficients
    void spline(int, double, double*, double*, double*, double*);

    //! Extract PDF value
    double getPDF(int, int, int, int, double) const;

    //! Extract PDF value for range difference
    double getPDFRange(int, int, int, int) const;

    //! Extract PDF value for transverse reduced chi-squared
    double getPDFRchisq(int, int, int, double) const;

    //! Pointer to the ExtManager singleton
    Simulation::ExtManager* m_extMgr;

    //! Pointer to the simulation's G4RunManager (if any)
    G4RunManager* m_runMgr;

    //! Pointer to the simulation's TrackingAction (if any)
    G4UserTrackingAction* m_trk;

    //! Pointer to the simulation's SteppingAction (if any)
    G4UserSteppingAction* m_stp;

    //! PDG code for the particle-ID hypotheses
    std::vector<int> m_pdgCode;

    //!  ChargedStable hypotheses
    std::vector<Const::ChargedStable> m_chargedStable;

    //! Pointers to geant4 physical volumes whose entry points will be saved
    std::vector<G4VPhysicalVolume*>* m_enter;

    //! Pointers to geant4 physical volumes whose exit points will be saved
    std::vector<G4VPhysicalVolume*>* m_exit;

    //! Time of flight (ns) along the track from the interaction point
    double m_tof;

    //! virtual "target" cylinder (boundary beyond which extrapolation ends)
    Simulation::ExtCylSurfaceTarget* m_target;

    //! offset (cm) along z axis of KLM midpoint from IP
    double m_OffsetZ;

    //! maximum radius (cm) of the endcaps
    double m_EndcapMaxR;

    //! minimum radius (cm) of the endcaps
    double m_EndcapMinR;

    //! mininum radius (cm) of the barrel
    double m_BarrelMinR;

    //! half-length (cm) of either endcap
    double m_EndcapHalfLength;

    //! half-length (cm) of the barrel
    double m_BarrelHalfLength;

    //! number of endcap layers
    int m_EndcapLayers;

    //! number of barrel layers
    int m_BarrelLayers;

    //! outermost active endcap layer
    int m_EndcapMaxLayer;

    //! midpoint along z (cm) of the forward endcap from the KLM midpoint
    double m_EndcapMiddleZ;

    //! uncertainty in measured strip position (cm)
    double m_StripPositionError;

    //! default measurement uncertainty (cm)
    double m_DefaultError;

    //! user-defined maximum distance (cm) for matching hit to extrapolation
    double m_maxDistCM;

    //! user-defined maximum distance (sigmas) for matching hit to extrapolation
    double m_maxDistSIGMA;

    //! RPC strip width (cm)
    double m_StripWidth;

    //! RPC strip position variance (cm^2)
    double m_StripPositionVariance;

    //! scintillator strip width (cm)
    double m_ScintWidth;

    //! scintillator strip position variance (cm^2)
    double m_ScintPositionVariance;

    //! minimum z (cm) of the active portion of the forward-BKLM RPCs relative to KLM midpoint
    double m_BarrelActiveMinZ;

    //! maximum z (cm) of the active portion of the forward-BKLM RPCs relative to KLM midpoint
    double m_BarrelActiveMaxZ;

    //! hit-plane radius (cm) at closest distance to IP of each barrel layer
    double m_BarrelModuleMiddleRadius[NLAYER + 1];

    //! hit-plane z (cm) of each IP layer relative to KLM midpoint
    double m_EndcapModuleMiddleZ[NLAYER + 1];

    //! minimum radius (r) of the active portion of the endcaps
    double m_EndcapActiveMinR;

    //! maximum radius (r) of the active portion of the endcaps
    double m_EndcapActiveMaxR;

    //! normal unit vector of each barrel sector
    TVector3 m_BarrelSectorPerp[NSECTOR + 1];

    //! rotation (radians) of each barrel sector relative to sector #0 (centred on the +x axis)
    TVector3 m_BarrelSectorPhi[NSECTOR + 1];

    //! flag to indicate that the extrapolation is outside the KLM
    bool m_leftKLM;

    //! flag to indicate that the extrapolation left the barrel and entered the endcap
    bool m_fromBarrelToEndcap;

    //! flag to indicate that the extrapolation left the barrel at +/-zmax and escaped
    int m_sideGapEscape;

    //! flag to indicate that the extrapolated track entered the barrel iron structure
    bool m_wasInBarrelIron;

    //! flag to indicate that the extrapolated track had been in the barrel in the prior step
    bool m_wasInBarrel;

    //! flag to indicate that the extrapolated track had been in the endcap in the prior step
    bool m_wasInEndcap;

    //! barrel layer number of the extrapolated track
    int m_geantBarrelLayer;

    //! endcap layer number of the extrapolated track
    int m_geantEndcapLayer;

    //! outermost barrel layer encountered by the extrapolated track in the prior steps
    int m_firstBarrelLayer;

    //! outermost barrel layer encountered by the extrapolated track in the prior steps
    int m_firstEndcapLayer;

    //! detector-element identifier at the current extrapolation step
    Address m_address;

    //! global position (cm) at the current extrapolation step
    TVector3 m_position;

    //! accumulated bit pattern of layers crossed by the extrapolated track
    int m_enteredPattern;

    //! accumulated bit pattern of layers with matching hits
    int m_matchedPattern;

    //! accumulated chi-squared of all in-plane transverse deviations between extrapolation and matching hit
    double m_chi2;

    //! accumulated number of points with matching 2D hits
    int m_nPoint;

    //! accumulated number of matching 1D strips (twice m_nPoint)
    int m_nStrips;

    //! outermost barrel layer crossed by the extrapolated track
    int m_lastBarrelLayerExt;

    //! outermost barrel layer with a matching hit
    int m_lastBarrelLayerHit;

    //! outermost endcap layer crossed by the extrapolated track
    int m_lastEndcapLayerExt;

    //! outermost endcap layer with a matching hit
    int m_lastEndcapLayerHit;

    //! number of barrel layers crossed by the extrapolated track
    int m_numBarrelLayerExt;

    //! number of barrel layers with matching hits
    int m_numBarrelLayerHit;

    //! number of endcap layers crossed by the extrapolated track
    int m_numEndcapLayerExt;

    //! number of endcap layers with matching hits
    int m_numEndcapLayerHit;

    //! probability density function for muon hypothesis
    MuidPar* m_muonPar;

    //! probability density function for pion hypothesis
    MuidPar* m_pionPar;

    //! probability density function for kaon hypothesis
    MuidPar* m_kaonPar;

  };

} // end of namespace Belle2

#endif // MUIDMODULE_H
