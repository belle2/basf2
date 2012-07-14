/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Leo Piilonen                                            *
 * Derived from:  G4ErrorPropagatorManager                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// It is the main interface for the user to define the setup and
// start the propagation. Initializes GEANT4 for the propagation.

#ifndef EXTMANAGER_H
#define EXTMANAGER_H

#include <globals.hh>
#include <G4ErrorPropagatorData.hh>
#include <G4ErrorPropagator.hh>
#include <G4ApplicationState.hh>

class G4ErrorRunManagerHelper;
class G4ErrorTarget;
class G4ErrorTrajState;

class G4VUserDetectorConstruction;
class G4VPhysicalVolume;
class G4VUserPhysicsList;
class G4UserRunAction;
class G4UserEventAction;
class G4UserStackingAction;
class G4UserTrackingAction;
class G4UserSteppingAction;
class G4Mag_UsualEqRhs;
class G4Track;

namespace Belle2 {

  class ExtNavigator;

  /** It is the main interface for the user to define the setup and start the propagation.
   *  Initializes GEANT4 for the propagation. **/
  class ExtManager {

  public:

    //! destructor
    ~ExtManager();

    //! Get pointer to the instance of this singleton class (create if needed)
    static ExtManager* GetManager();

    //! Terminate an event and set state to G4ErrorState_Init
    void EventTermination();

    //! Terminate a run and set state to G4ErrorState_Init
    void RunTermination();

    //! Initialize Geant4 and Geant4e
    void InitGeant4e();

    //! Initialize for propagation of a track and set state to G4ErrorState_Propagating
    void InitTrackPropagation();

    //! Create G4ErrorMag_UsualEqRhs for backward propagation of a track
    G4bool InitFieldForBackwards();

    //! Propagate a track to completion
    G4int Propagate(G4ErrorTrajState* currentTS, const G4ErrorTarget* target, G4ErrorMode mode = G4ErrorMode_PropForwards);

    //! Propagate a track by one step
    G4int PropagateOneStep(G4ErrorTrajState* currentTS,
                           G4ErrorMode mode = G4ErrorMode_PropForwards);

    //! Invoke G4ErrorRunManagerHelper to construct detector and set world volume
    void SetUserInitialization(G4VUserDetectorConstruction*);

    //! Invoke G4ErrorRunManagerHelper to set world volume
    void SetUserInitialization(G4VPhysicalVolume*);

    //! Invoke G4ErrorRunManagerHelper to initialize ext-specific physics
    void SetUserInitialization(G4VUserPhysicsList*);

    //! Tell G4EventManager about our tracking action
    void SetUserAction(G4UserTrackingAction*);

    //! Tell G4EventManager about our stepping action
    void SetUserAction(G4UserSteppingAction*);

    //! Print the extrapolator's state
    G4String PrintExtState();

    //! Print the extrapolator's state
    G4String PrintExtState(G4ErrorState);

    //! Print the geant4 state
    G4String PrintG4State();

    //! Print the geant4 state
    G4String PrintG4State(G4ApplicationState);

    //! Get the helper
    inline G4ErrorRunManagerHelper* GetHelper() const;

    //! Set the verbosity level for the extrapolator's stepping manager
    void SetSteppingManagerVerboseLevel();

    //! Get the propagator
    inline G4ErrorPropagator* GetPropagator() const;

    //! Get the navigator
    inline ExtNavigator* GetNavigator() const;

  private:

    //! constructor is hidden: user calls ExtManager::GetManager() instead
    ExtManager();

    //! Create a RunManager helper
    void StartHelper();

    //! create an ExtNavigator
    void StartNavigator();

  private:

    //! Stores pointer to the singleton class
    static ExtManager* m_manager;

    //! Stores pointer to the RunManager helper
    G4ErrorRunManagerHelper* m_helper;

    //! Stores pointer to the propagator
    G4ErrorPropagator* m_propagator;

    //! Stores pointer to the navigator
    ExtNavigator* m_navigator;

    //! Stores pointer to the backward-propagation EquationOfMotion
    G4Mag_UsualEqRhs* m_equationOfMotion;

  };

  inline G4ErrorRunManagerHelper* ExtManager::GetHelper() const { return m_helper; }

  inline G4ErrorPropagator* ExtManager::GetPropagator() const { return m_propagator; }

  inline ExtNavigator* ExtManager::GetNavigator() const { return m_navigator; }

} // end of namespace Belle2

#endif // EXTMANAGER_H
