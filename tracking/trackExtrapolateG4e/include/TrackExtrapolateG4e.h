/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRACKEXTRAPOLATEG4E_H
#define TRACKEXTRAPOLATEG4E_H

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <tracking/dataobjects/ExtHit.h>
#include <bklm/geometry/GeometryPar.h>

#include <G4TouchableHandle.hh>
#include <G4ErrorTrajErr.hh>
#include <G4ThreeVector.hh>

#include <string>
#include <vector>

class G4ErrorSymMatrix;
class G4VPhysicalVolume;
class G4ErrorFreeTrajState;
class G4StepPoint;

namespace Belle2 {

  class Track;
  class RecoTrack;
  class Muid;
  class MuidPar;
  class KLMCluster;
  class ECLCluster;
  namespace Simulation {
    class ExtCylSurfaceTarget;
    class ExtManager;
  }

  //! Enumeration for G4VPhysicalVolume sensitive-volume categories
  enum VolTypes {
    //! CDC
    VOLTYPE_CDC,
    //! TOP container
    VOLTYPE_TOP1,
    //! TOP quartz
    VOLTYPE_TOP2,
    //! TOP glue
    VOLTYPE_TOP3,
    //! ARICH aerogel
    VOLTYPE_ARICH1,
    //! ARICH Img plate
    VOLTYPE_ARICH2,
    //! ARICH HAPD window
    VOLTYPE_ARICH3,
    //! ECL
    VOLTYPE_ECL,
    //! BKLM RPC
    VOLTYPE_BKLM1,
    //! BKLM scintillator
    VOLTYPE_BKLM2,
    //! EKLM
    VOLTYPE_EKLM
  };

  //! Data structure to define extrapolation state
  struct ExtState {
    //! Pointer to the reconstructed track
    const Track* track;
    //! Particle hypothesis that is being extrapolated
    int pdgCode;
    //! True for back-propagation of a cosmic ray
    bool isCosmic;
    //! Time of flight from IP (ns), updated during extrapolation
    double tof;
    //! Length from start of extrapolation (rad lengths), updated during extrapolation
    double length;
    //! MUID: initial direction of track, used for KLID
    G4ThreeVector directionAtIP;
    //! MUID: accumulated chi-squared of all in-plane transverse deviations between extrapolation and matching hit
    double chi2;
    //! MUID: accumulated number of points with matching 2D hits
    int nPoint;
    //! MUID: outermost barrel layer encountered by the extrapolated track in the prior steps
    int firstBarrelLayer;
    //! MUID: outermost barrel layer encountered by the extrapolated track in the prior steps
    int firstEndcapLayer;
    //! MUID: outermost barrel layer crossed by the extrapolated track
    int lastBarrelExtLayer;
    //! MUID: outermost barrel layer with a matching hit
    int lastBarrelHitLayer;
    //! MUID: outermost endcap layer crossed by the extrapolated track
    int lastEndcapExtLayer;
    //! MUID: outermost endcap layer with a matching hit
    int lastEndcapHitLayer;
    //! MUID: accumulated bit pattern of layers crossed by the extrapolated track
    int extLayerPattern;
    //! MUID: accumulated bit pattern of layers with matching hits
    int hitLayerPattern;
    //! MUID: flag to indicate that the extrapolated track escaped from the KLM
    bool escaped;
  };

  //! intersection of muid-extrapolated track with a KLM layer
  struct Intersection {
    //! index in {B,E}KLMHit2ds of matching hit
    int hit;
    //! flag to indicate if this point is in the barrel (true) or endcap (false)
    bool inBarrel;
    //! flag to indicate if this point is in the forward (true) or backward (false) end
    bool isForward;
    //! sector number (0..7 for barrel, 0..3 for endcap) of this point
    int sector;
    //! layer number (0..14 for barrel, 0..13 for endcap) of this point
    int layer;
    //! extrapolated-track global position (cm) of this intersection
    G4ThreeVector position;
    //! extrapolated-track momentum (GeV/c) at this intersection
    G4ThreeVector momentum;
    //! extrapolated-track phase-space covariance matrix at this intersection
    G4ErrorSymMatrix covariance;
    //! extrapolated-track position (cm) projected to the 2D hit's midplane
    G4ThreeVector positionAtHitPlane;
    //! time (ns) of matching BKLMHit2d
    double time;
    //! chi-squared value of transverse deviation between extrapolated and measured hit positions
    double chi2;
  };

  /** geant4e-based track extrapolation
     *
     * This class extrapolates tracks outward from the outer perimeter of the CDC
     * using geant4e.
     *
     * This class requires a valid geometry in memory (gGeoManager). Therefore,
     * a geometry building module should have been executed before this module is called.
     *
     * This class has the same functions as a module - and these are called from
     * the ExtModule's so-named functions - but also has an entry that can be
     * called to extrapolate a single user-defined track.
     *
     */

  class TrackExtrapolateG4e {

  public:

    //! Get the singleton's address
    static TrackExtrapolateG4e* getInstance();

    //! destructor
    ~TrackExtrapolateG4e();

    //! Assign the Tracks collection name before initialization
    void setTracksColName(std::string& tracksColName) { m_TracksColName = &tracksColName; }

    //! Assign the RecoTracks collection name before initialization
    void setRecoTracksColName(std::string& recoTracksColName) { m_RecoTracksColName = &recoTracksColName; }

    //! Assign the ExtHits collection name before initialization
    void setExtHitsColName(std::string& extHitsColName) { m_ExtHitsColName = &extHitsColName; }

    //! Assign the Muids collection name before initialization
    void setMuidsColName(std::string& muidsColName) { m_MuidsColName = &muidsColName; }

    //! Assign the MuidHits collection name before initialization
    void setMuidHitsColName(std::string& muidHitsColName) { m_MuidHitsColName = &muidHitsColName; }

    //! Assign the BKLMHits collection name before initialization
    void setBKLMHitsColName(std::string& bklmHitsColName) { m_BKLMHitsColName = &bklmHitsColName; }

    //! Assign the EKLMHits collection name before initialization
    void setEKLMHitsColName(std::string& eklmHitsColName) { m_EKLMHitsColName = &eklmHitsColName; }

    //! Assign the KLMClusters collection name before initialization
    void setKLMClustersColName(std::string& klmClustersColName) { m_KLMClustersColName = &klmClustersColName; }

    //! Assign the ECLClusters collection name before initialization
    void setECLClustersColName(std::string& eclClustersColName) { m_ECLClustersColName = &eclClustersColName; }

    //! Assign the TrackClusterSeparations collection name before initialization
    void setTrackClusterSeparationsColName(std::string& trackClusterSeparationsColName) { m_TrackClusterSeparationsColName = &trackClusterSeparationsColName; }

    //! Initialize for track extrapolation by the EXT module.
    //! @param minPt Minimum transverse momentum to begin extrapolation (GeV/c).
    //! @param minKE Minimum kinetic energy to continue extrapolation (GeV/c).
    //! @param hypotheses Vector of charged-particle hypotheses used in extrapolation of each track.
    void initialize(double minPt, double minKE,
                    std::vector<Const::ChargedStable>& hypotheses);

    //! Initialize for track extrapolation by the MUID module.
    //! @param meanDt Mean value of the in-time window (ns).
    //! @param maxDt Half-width of the in-time window (ns).
    //! @param maxSeparation Maximum separation between track crossing and matching hit in detector plane (#sigmas).
    //! @param maxKLMTrackClusterDistance Maximum distance between associated track and KLMCluster (cm), criterion for matching relation Track->KLMCluster on MDST.
    //! @param maxECLTrackClusterDistance Maximum distance between associated track and ECLCluster (cm).
    //! @param minPt Minimum transverse momentum to begin extrapolation (GeV/c).
    //! @param minKE Minimum kinetic energy to continue extrapolation (GeV/c).
    //! @param hypotheses Vector of charged-particle hypotheses used in extrapolation of each track.
    void initialize(double meanDt, double maxDt, double maxSeparation,
                    double maxKLMTrackClusterDistance, double maxECLTrackClusterDistance,
                    double minPt, double minKE, std::vector<Const::ChargedStable>& hypotheses);

    //! Perform beginning-of-run actions.
    //! @param flag True if called by Muid module, false if called by Ext module.
    void beginRun(bool flag);

    //! Performs track extrapolation for all tracks in one event.
    //! @param flag True if called by Muid module, false if called by Ext module.
    void event(bool flag);

    //! Perform end-of-run actions.
    //! @param flag True if called by Muid module, false if called by Ext module.
    void endRun(bool flag);

    //! Terminates this singleton.
    //! @param flag True if called by Muid module, false if called by Ext module.
    void terminate(bool flag);

    //! Performs track extrapolation for a single track (specified in genfit2 units).
    //! @param pdgCode Signed PDG identifier of the particle hypothesis to be used for the extrapolation.
    //! @param tof Starting time, i.e., time of flight from the IP, at the starting point (ns).
    //! @param position Starting point of the extrapolation (cm).
    //! @param momentum Momentum of the track at the starting point (GeV/c).
    //! @param covariance Phase-space covariance matrix (6x6) at the starting point (cm, GeV/c).
    //! @param extHitsColName (not used).
    void extrapolate(int pdgCode,
                     double tof,
                     const G4ThreeVector& position,
                     const G4ThreeVector& momentum,
                     const G4ErrorSymMatrix& covariance,
                     const std::string& extHitsColName);

    //! Performs muon identification for a single track (specified in genfit2 units).
    //! @param pdgCode Signed PDG identifier of the particle hypothesis to be used for the extrapolation.
    //! @param tof Starting time, i.e., time of flight from the IP, at the starting point (ns).
    //! @param isCosmic True to back-extrapolate a cosmic ray
    //! @param position Starting point of the extrapolation (cm).
    //! @param momentum Momentum of the track at the starting point (GeV/c).
    //! @param covariance Phase-space covariance matrix (6x6) at the starting point (cm, GeV/c).
    void identifyMuon(int pdgCode,
                      double tof,
                      bool isCosmic,
                      const G4ThreeVector& position,
                      const G4ThreeVector& momentum,
                      const G4ErrorSymMatrix& covariance);

  private:

    //! constructor is hidden; user calls TrackExtrapolateG4e::getInstance() instead
    TrackExtrapolateG4e();

    //! copy constructor is hidden; user calls TrackExtrapolateG4e::getInstance() instead
    TrackExtrapolateG4e(TrackExtrapolateG4e&);

    //! Swim a single track (MUID) until it stops or leaves the target cylinder
    void swim(ExtState&, G4ErrorFreeTrajState&,
              const std::vector<std::pair<ECLCluster*, G4ThreeVector> >*,
              const std::vector<std::pair<KLMCluster*, G4ThreeVector> >*,
              std::vector<std::map<const Track*, double> >*);

    //! Swim a single track (EXT) until it stops or leaves the target cylinder
    void swim(ExtState&, G4ErrorFreeTrajState&);

    //! Register the list of geant4 physical volumes whose entry/exit
    //! points will be saved during extrapolation
    void registerVolumes();

    //! Get the physical volume information for a geant4 physical volume
    void getVolumeID(const G4TouchableHandle&, Const::EDetector&, int&);

    //! Convert the geant4e 5x5 covariance to phasespace 6x6 covariance
    void fromG4eToPhasespace(const G4ErrorFreeTrajState&, G4ErrorSymMatrix&);

    //! Convert the phasespace covariance to geant4e covariance
    void fromPhasespaceToG4e(const G4ThreeVector&, const G4ErrorSymMatrix&, G4ErrorTrajErr&);

    //! Convert the phasespace covariance to geant4e covariance
    void fromPhasespaceToG4e(const TVector3&, const TMatrixDSym&, G4ErrorTrajErr&);

    //! Get the start point for a new reconstructed track with specific PDG hypothesis
    ExtState getStartPoint(const Track&, int, G4ErrorFreeTrajState&);

    //! Create another EXT extrapolation hit for a track candidate
    void createExtHit(ExtHitStatus, const ExtState&, const G4ErrorFreeTrajState&, const G4StepPoint*, const G4TouchableHandle&);

    //! Create another EXT ECL-crystal-crossing hit for a track candidate
    void createECLHit(const ExtState&, const G4ErrorFreeTrajState&, const G4StepPoint*, const G4StepPoint*, const G4TouchableHandle&,
                      const std::pair<ECLCluster*, G4ThreeVector>&, double, double);

    //! Create another MUID extrapolation hit for a track candidate
    bool createMuidHit(ExtState&, G4ErrorFreeTrajState&, std::vector<std::map<const Track*, double> >*);

    //! Find the intersection point of the track with the crossed BKLM plane
    bool findBarrelIntersection(ExtState&, const G4ThreeVector&, Intersection&);

    //! Find the intersection point of the track with the crossed EKLM plane
    bool findEndcapIntersection(ExtState&, const G4ThreeVector&, Intersection&);

    //! Find the matching BKLM 2D hit nearest the intersection point of the track with the crossed BKLM plane
    bool findMatchingBarrelHit(Intersection&, const Track*);

    //! Find the matching EKLM 2D hit nearest the intersection point of the track with the crossed EKLM plane
    bool findMatchingEndcapHit(Intersection&, const Track*);

    //! Nudge the track using the matching hit
    void adjustIntersection(Intersection&, const double*, const G4ThreeVector&, const G4ThreeVector&);

    //! Complete muon identification after end of track extrapolation
    void finishTrack(const ExtState&, Muid*, bool);

    //! Stores pointer to the singleton class
    static TrackExtrapolateG4e* m_Singleton;

    //! Flag to indicate that EXT initialize() has been called
    bool m_ExtInitialized;

    //! Flag to indicate that MUID initialize() has been called
    bool m_MuidInitialized;

    //! Mean hit - trigger time (ns)
    double m_MeanDt;

    //! Coincidence window half-width for in-time KLM hits (ns)
    double m_MaxDt;

    //! Magnetic field z component (gauss) at origin
    double m_MagneticField;

    //! user-defined maximum squared-distance (#variances) for matching hit to extrapolation
    double m_MaxDistSqInVariances;

    //! user-defined maximum distance (mm) between KLMCluster and associated track (for KLID)
    double m_MaxKLMTrackClusterDistance;

    //! user-defined maximum distance (mm) between ECLCluster and associated track (for EID)
    double m_MaxECLTrackClusterDistance;

    //! Minimum transverse momentum in MeV/c for extrapolation to be started
    double m_MinPt;

    //! Minimum kinetic energy in MeV for extrapolation to continue
    double m_MinKE;

    //! Pointer to name of the Track collection of the reconstructed tracks to be extrapolated
    std::string* m_TracksColName;

    //! Pointer to name of the RecoTrack collection of the reconstructed tracks to be extrapolated
    std::string* m_RecoTracksColName;

    //! Pointer to name of the extHit collection of the extrapolation hits
    std::string* m_ExtHitsColName;

    //! Pointer to name of the muid collection of the muon identification information
    std::string* m_MuidsColName;

    //! Pointer to name of the muidHit collection of the extrapolation hits
    std::string* m_MuidHitsColName;

    //! Pointer to name of the BKLM 2D hits collection
    std::string* m_BKLMHitsColName;

    //! Pointer to name of the EKLM 2D hits collection
    std::string* m_EKLMHitsColName;

    //! Pointer to name of the KLMCluster collection
    std::string* m_KLMClustersColName;

    //! Pointer to name of the ECLCluster collection
    std::string* m_ECLClustersColName;

    //! Pointer to name of the TrackClusterSeparation collection
    std::string* m_TrackClusterSeparationsColName;

    //! Pointer to the ExtManager singleton
    Simulation::ExtManager* m_ExtMgr;

    //!  ChargedStable hypotheses for EXT
    const std::vector<Const::ChargedStable>* m_HypothesesExt;

    //!  ChargedStable hypotheses for MUID
    const std::vector<Const::ChargedStable>* m_HypothesesMuid;

    //! Pointer to an empty string
    std::string* m_DefaultName;

    //! Default ChargedStable hypotheses (needed as call argument but not used)
    std::vector<Const::ChargedStable>* m_DefaultHypotheses;

    //! Pointers to geant4 physical volumes whose entry/exit points will be saved
    std::map<G4VPhysicalVolume*, enum VolTypes>* m_EnterExit;

    //! Pointers to BKLM geant4 sensitive (physical) volumes
    std::vector<G4VPhysicalVolume*>* m_BKLMVolumes;

    //! Pointers to EKLM geant4 sensitive (physical) volumes
    std::vector<G4VPhysicalVolume*>* m_EKLMVolumes;

    //! virtual "target" cylinder for EXT (boundary beyond which extrapolation ends)
    Simulation::ExtCylSurfaceTarget* m_TargetExt;

    //! virtual "target" cylinder for MUID (boundary beyond which extrapolation ends)
    Simulation::ExtCylSurfaceTarget* m_TargetMuid;

    //! Minimum squared radius (cm) outside of which extrapolation will continue
    double m_MinRadiusSq;

    //! offset (cm) along z axis of KLM midpoint from IP
    double m_OffsetZ;

    //! Number of barrel sectors
    int m_BarrelNSector;

    //! maximum radius (cm) of the barrel
    double m_BarrelMaxR;

    //! minimum radius (cm) of the barrel
    double m_BarrelMinR;

    //! half-length (cm) of the barrel
    double m_BarrelHalfLength;

    //! outermost barrel layer that is active for muon identification (user-defined)
    int m_OutermostActiveBarrelLayer;

    //! BKLM RPC phi-measuring strip position variance (cm^2) by layer
    double m_BarrelPhiStripVariance[NLAYER + 1];

    //! BKLM RPC z-measuring strip position variance (cm^2) by layer
    double m_BarrelZStripVariance[NLAYER + 1];

    //! BKLM scintillator strip position variance (cm^2)
    double m_BarrelScintVariance;

    //! hit-plane radius (cm) at closest distance to IP of each barrel end | sector | layer
    double m_BarrelModuleMiddleRadius[2][NSECTOR + 1][NLAYER + 1];

    //! normal unit vector of each barrel sector
    G4ThreeVector m_BarrelSectorPerp[NSECTOR + 1];

    //! azimuthal unit vector of each barrel sector
    G4ThreeVector m_BarrelSectorPhi[NSECTOR + 1];

    //! maximum radius (cm) of the endcaps
    double m_EndcapMaxR;

    //! minimum radius (cm) of the endcaps
    double m_EndcapMinR;

    //! midpoint along z (cm) of the forward endcap from the KLM midpoint
    double m_EndcapMiddleZ;

    //! half-length (cm) of either endcap
    double m_EndcapHalfLength;

    //! outermost forward-endcap layer that is active for muon identification (user-defined)
    int m_OutermostActiveForwardEndcapLayer;

    //! outermost backward-endcap layer that is active for muon identification (user-defined)
    int m_OutermostActiveBackwardEndcapLayer;

    //! EKLM scintillator strip position variance (cm^2)
    double m_EndcapScintVariance;

    //! hit-plane z (cm) of each IP layer relative to KLM midpoint
    double m_EndcapModuleMiddleZ[NLAYER + 1];

    //! experiment number for the current set of particle-hypothesis PDFs
    int m_ExpNo;

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

#endif // TRACKEXTRAPOLATEG4E_H
