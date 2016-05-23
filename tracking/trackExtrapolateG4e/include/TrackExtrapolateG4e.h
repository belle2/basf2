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
#include <tracking/dataobjects/ExtHit.h>
#include <framework/gearbox/Const.h>
#include <simulation/kernel/ExtCylSurfaceTarget.h>

#include <G4TouchableHandle.hh>
#include <G4ErrorTrajErr.hh>
#include <TMatrixDSym.h>

#include <string>
#include <vector>

class G4ErrorPropagator;
class G4ErrorRunManagerHelper;
class G4RunManager;
class G4MagneticField;
class G4Mag_UsualEqRhs;
class G4MagIntegratorStepper;
class G4ChordFinder;
class G4VisManager;
class G4VUserPhysicsList;
class G4UserTrackingAction;
class G4UserSteppingAction;
class G4VPhysicalVolume;
class G4ErrorFreeTrajState;
class G4String;
namespace genfit { class Track; }

namespace Belle2 {

  class Track;
  namespace Simulation { class ExtManager; }
  struct ExtState {
    bool isMuid;
    int pdgCode;
    G4ErrorFreeTrajState* g4eState;
    Track* track;
    const std::string& extHitsColName;
    double tof;
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
    static TrackExtrapolateG4e* GetInstance();

    //! destructor
    ~TrackExtrapolateG4e();

    //! Initialize for track extrapolation by the ext module
    void initialize(const std::vector<int>& pdgCodes,
                    const std::string& tracksColName,
                    const std::string& extHitsColName,
                    double minPt,
                    double minKE,
                    double maxStep,
                    int cosmic,
                    int trackingVerbosity,
                    bool enableVisualization,
                    const std::string& magneticFieldStepperName,
                    double magneticCacheDistance,
                    double deltaChordInMagneticField,
                    const std::vector<std::string> uiCommands);

    //! Perform beginning-of-run actions
    void beginRun();

    //! Performs track extrapolation for all tracks in one event
    //! flag is true if called by Muid, false if called by Ext
    void event(bool);

    //! Performs track extrapolation for a single track (specified in genfit2 units: cm, GeV/c)
    void extrapolate(int pdgCode,
                     double tof,
                     const G4ThreeVector& position,
                     const G4ThreeVector& momentum,
                     const G4ErrorSymMatrix& covariance, // (6x6)
                     const std::string& extHitsColName);

    //! Perform end-of-run actions
    void endRun();

    //! Terminates this singleton
    void terminate();

  private:

    //! constructor is hidden; user calls TrackExtrapolateG4e::GetInstance() instead
    TrackExtrapolateG4e();

    //! copy constructor is hidden; user calls TrackExtrapolateG4e::GetInstance() instead
    TrackExtrapolateG4e(TrackExtrapolateG4e&);

    //! Register the list of geant4 physical volumes whose entry/exit
    //! points will be saved during extrapolation
    void registerVolumes();

    //! Get the physical volume information for a geant4 physical volume
    void getVolumeID(const G4TouchableHandle&, Const::EDetector&, int&);

    //! Convert the geant4e 5x5 covariance to phasespace 6x6 covariance
    TMatrixDSym fromG4eToPhasespace(const G4ErrorFreeTrajState*);

    //! Convert the phasespace covariance to geant4e covariance
    G4ErrorTrajErr fromPhasespaceToG4e(const TVector3&, const TMatrixDSym&);

    //! Convert the phasespace covariance to geant4e covariance
    G4ErrorTrajErr fromPhasespaceToG4e(const G4ThreeVector&, const G4ErrorSymMatrix&);

    //! Define a new track candidate for one reconstructed track and PDG hypothesis
    void getStartPoint(const genfit::Track*, int, G4ThreeVector&, G4ThreeVector&, G4ErrorTrajErr&, double&);

    //! Create another extrapolation hit for a track candidate
    void createHit(ExtHitStatus, ExtState&);

//    //! Create another extrapolation hit for a single-track extrapolation
//    void createHitForOneTrack(const G4ErrorFreeTrajState*, ExtHitStatus, const std::string&, int);

    //! Stores pointer to the singleton class
    static TrackExtrapolateG4e* m_Singleton;

    //! Minimum transverse momentum in MeV/c for extrapolation to be started
    double m_MinPt;

    //! Minimum kinetic energy in MeV for extrapolation to continue
    double m_MinKE;

    //! Flag to enable cosmic-ray extrapolation (i.e., from outside toward IP in top half of detector)
    int m_Cosmic;

    //! Name of the Track collection of the reconstructed tracks to be extrapolated
    std::string m_TracksColName;

    //! Name of the extHit collection of the extrapolation hits
    std::string m_ExtHitsColName;

    //! Pointer to the ExtManager singleton
    Simulation::ExtManager* m_ExtMgr;

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

  };

} // end of namespace Belle2

#endif // TRACKEXTRAPOLATEG4E_H
