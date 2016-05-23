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
  class TrackExtrapolateG4e;
  namespace Simulation { class ExtManager; }

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

    //! User-selected PDG codes to extrapolate (anti-particles are included implicitly)
    std::vector<int> m_PDGCodes;

    //! Minimum transverse momentum in GeV/c for extrapolation to be started
    double m_MinPt;

    //! Minimum kinetic energy in GeV for extrapolation to continue
    double m_MinKE;

    //! Maximum step size in cm (0 for no upper limit)
    double m_MaxStep;

    //! Flag for source (0 = beam, 1 = cosmic ray)
    int    m_Cosmic;

    //! Name of the Track collection of the reconstructed tracks to be extrapolated
    std::string m_TracksColName;

    //! Name of the extHit collection of the extrapolation hits
    std::string m_ExtHitsColName;

    //! Tracking verbosity: 0=Silent; 1=Min info per step; 2=sec particles; 3=pre/post step info; 4=like 3 but more info; 5=proposed step length info
    int m_TrackingVerbosity;

    //! A list of Geant4 UI commands that should be applied before the extrapolation starts
    std::vector<std::string> m_UICommands;

    //! If set to true the Geant4 visualization support is enabled
    bool m_EnableVisualization;

    //! magnetic field stepper to use
    std::string m_MagneticFieldStepperName;

    //! minimal distance for magnetic field lookup. If distance is smaller, return cached value
    double m_MagneticCacheDistance;

    //! maximum miss-distance between the trajectory curve and its linear chord(s) approximation
    double m_DeltaChordInMagneticField;

  private:

    TrackExtrapolateG4e* m_Extrapolator;

  };

} // end of namespace Belle2

#endif // EXTMODULE_H
