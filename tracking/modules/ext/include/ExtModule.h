/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EXTMODULE_H
#define EXTMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <tracking/dataobjects/ExtHit.h>
#include <framework/gearbox/Const.h>

#include <G4TouchableHandle.hh>
#include <G4ErrorTrajErr.hh>
#include <TMatrixD.h>

#include <string>
#include <vector>

class G4RunManager;
class G4VUserPhysicsList;
class G4UserTrackingAction;
class G4UserSteppingAction;
class G4VPhysicalVolume;
class G4ErrorFreeTrajState;
class G4String;
class GFTrack;

namespace Belle2 {

  class ExtManager;
  class ExtCylSurfaceTarget;

  /** The geant4e-based track extrapolation module.
   *
   * This module extrapolates tracks outward from the outer perimeter of the CDC
   * using geant4e.
   *
   * This module requires a valid geometry in memory (gGeoManager). Therefore,
   * a geometry building module should have been executed before this module is called.
   *
   */

  class ExtModule : public Module {

  public:

    //! constructor
    ExtModule();

    //! destructor
    virtual ~ExtModule();

    //! Initialize for track extrapolation
    virtual void initialize();

    //! Perform beginning-of-run actions
    virtual void beginRun();

    //! Performs track extrapolation for all tracks in one event
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
    std::string m_TracksColName;

    //! Name of the extHit collection of the extrapolation hits
    std::string m_extHitsColName;

  private:

    //! Register the list of geant4 physical volumes whose entry/exit
    //! points will be saved during extrapolation
    void registerVolumes();

    //! Get the physical volume information for a geant4 physical volume
    void getVolumeID(const G4TouchableHandle&, ExtDetectorID&, int&);

    //! Convert the geant4e covariance to phasespacePoint 6x6 covariance
    TMatrixD getCov(const G4ErrorFreeTrajState*);

    //! Define a new track candidate for one reconstructed track and PDG hypothesis
    void getStartPoint(const GFTrack*, int, G4ThreeVector&, G4ThreeVector&, G4ErrorTrajErr&);

    //! Create another extrapolation hit for a track candidate
    void createHit(const G4ErrorFreeTrajState*, ExtHitStatus, int, int, StoreArray<ExtHit>&, RelationArray&);

    //! Pointer to the ExtManager singleton
    ExtManager* m_extMgr;

    //! Pointer to the simulation's G4RunManager (if any)
    G4RunManager* m_runMgr;

    //! Pointer to the simulation's TrackingAction (if any)
    G4UserTrackingAction* m_trk;

    //! Pointer to the simulation's SteppingAction (if any)
    G4UserSteppingAction* m_stp;

    //! PDG codes for the particleID hypotheses
    std::vector<int> m_pdgCode;

    //!  ChargedStable hypotheses
    std::vector<Const::ChargedStable> m_chargedStable;

    // Pointers to geant4 physical volumes whose entry/exit points will be saved
    std::vector<G4VPhysicalVolume*>* m_enter;
    std::vector<G4VPhysicalVolume*>* m_exit;

    // Time of flight (ns) along the track from the interaction point
    double m_tof;

    // virtual "target" cylinder
    ExtCylSurfaceTarget* m_target;

  };

} // end of namespace Belle2

#endif // EXTMODULE_H
