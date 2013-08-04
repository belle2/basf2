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
#include <simulation/kernel/ExtManager.h>
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
class GFTrack;

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

  //! detector-element identification
  struct Address {
    bool isRPC;
    bool inBarrel;
    bool isForward;
    bool isInnerPlane;
    int  sector;
    int  layer;
  };

  // possible intersection of extrapolated track with a KLM layer
  struct Point {
    bool                enteredSensitiveVolume;
    int                 intersected;
    bool                hasMatchingHit;
    Address             address;
    TVector3            position;
    TVector3            momentum;
    CLHEP::HepSymMatrix covariance;
    TVector3            positionAtHitPlane;
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

    //! Name of the GFTrack collection of the reconstructed tracks to be extrapolated
    std::string m_gfTracksColName;

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
    void getStartPoint(const GFTrack*, int, G4ThreeVector&, G4ThreeVector&, G4ErrorTrajErr&);

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

    //! Convert GEANT4 physical volume name in BKLM/EKLM to an Address
    void getAddress(const G4String&, Address&);

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

    // Pointers to geant4 physical volumes whose entry/exit points will be saved
    std::vector<G4VPhysicalVolume*>* m_enter;
    std::vector<G4VPhysicalVolume*>* m_exit;

    // Time of flight (ns) along the track from the interaction point
    double m_tof;

    double m_OffsetZ;
    double m_EndcapMaxR;
    double m_EndcapMinR;
    double m_BarrelMinR;
    double m_EndcapHalfLength;
    double m_BarrelHalfLength;
    int m_EndcapLayers;
    int m_BarrelLayers;
    int m_EndcapMaxLayer;
    double m_EndcapMiddleZ;
    double m_StripPositionError;
    double m_DefaultError;
    double m_maxDistCM;
    double m_maxDistSIGMA;
    double m_StripWidth;
    double m_StripPositionVariance;
    double m_ScintWidth;
    double m_ScintPositionVariance;
    double m_BarrelActiveMinZ;
    double m_BarrelActiveMaxZ;
    double m_BarrelModuleMiddleRadius[NLAYER + 1];
    double m_EndcapModuleMiddleZ[NLAYER + 1];
    double m_EndcapActiveMinR;
    double m_EndcapActiveMaxR;
    TVector3 m_BarrelSectorPerp[NSECTOR + 1];
    TVector3 m_BarrelSectorPhi[NSECTOR + 1];
    bool m_leftKLM;
    bool m_fromBarrelToEndcap;
    int m_sideGapEscape;
    bool m_wasInBarrelIron;
    bool m_wasInBarrel;
    bool m_wasInEndcap;
    int m_geantBarrelLayer;
    int m_geantEndcapLayer;
    int m_firstBarrelLayer;
    int m_firstEndcapLayer;
    Address m_address;
    TVector3 m_position;
    int m_enteredPattern;
    int m_matchedPattern;
    double m_chi2;
    int m_nPoint;
    int m_nStrips;
    int m_lastBarrelLayerExt;
    int m_lastBarrelLayerHit;
    int m_lastEndcapLayerExt;
    int m_lastEndcapLayerHit;
    int m_numBarrelLayerExt;
    int m_numBarrelLayerHit;
    int m_numEndcapLayerExt;
    int m_numEndcapLayerHit;

    MuidPar* m_muonPar;
    MuidPar* m_pionPar;
    MuidPar* m_kaonPar;

  };

} // end of namespace Belle2

#endif // MUIDMODULE_H
