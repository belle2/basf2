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
#include <G4ApplicationState.hh>
#include <G4ErrorPropagatorData.hh>
#include <G4ErrorPropagator.hh>

class G4ErrorTarget;
class G4ErrorTrajState;
class G4RunManager;
class G4VUserDetectorConstruction;
class G4VPhysicalVolume;
class G4VUserPhysicsList;
class G4UserRunAction;
class G4UserEventAction;
class G4UserStackingAction;
class G4UserTrackingAction;
class G4UserSteppingAction;
class G4MagneticField;
class G4MagIntegratorStepper;
class G4EquationOfMotion;
class G4ChordFinder;
class G4Mag_UsualEqRhs;
class G4VisManager;
class G4Track;
class G4VExceptionHandler;

namespace Belle2 {

  namespace Simulation {

    /** It is the main interface for the user to define the setup and start the propagation.
     *  Initializes GEANT4 for the propagation. **/
    class ExtManager {

    public:

      //! destructor
      ~ExtManager();

      //! Get pointer to the instance of this singleton class (create if needed)
      static ExtManager* GetManager();

      //! Terminate an event and set state to G4ErrorState_Init
      void EventTermination(G4ErrorMode);

      //! Terminate a run and set state to G4ErrorState_Init
      void RunTermination();

      //! Initialize Geant4 and Geant4e
      void Initialize(const char [], const std::string&, double, double, bool, int, const std::vector<std::string>&);

      //! Initialize for propagation of a track and set state to G4ErrorState_Propagating
      void InitTrackPropagation(G4ErrorMode);

      //! Propagate a track by one step
      G4int PropagateOneStep(G4ErrorTrajState* currentTS, G4ErrorMode mode = G4ErrorMode_PropForwards);

      //! Get the propagator
      inline G4ErrorPropagator* GetPropagator() const { return m_Propagator; }

    private:

      //! constructor is hidden: user calls ExtManager::GetManager() instead
      ExtManager();

      //! copy constructor is undefined for singleton; user calls ExtManager::GetManager() instead
      ExtManager(ExtManager&) = delete;

      //! Assignment operator is undefined for singleton; user calls ExtManager::GetManager() instead
      ExtManager& operator=(ExtManager&) = delete;

    private:

      //! Stores pointer to the singleton class
      static ExtManager* m_Manager;

      //! Initial state of the G4RunManager (=PreInitif FullSimModule not present)
      G4ApplicationState m_G4State;

      //! Stores pointer to the propagator
      G4ErrorPropagator* m_Propagator;

      //! Pointer to the simulation's G4RunManager (if any)
      G4RunManager* m_G4RunMgr;

      //! Pointer to the simulation's TrackingAction (if any)
      G4UserTrackingAction* m_TrackingAction;

      //! Pointer to the simulation's SteppingAction (if any)
      G4UserSteppingAction* m_SteppingAction;

      //! Pointer to the G4VExceptionHandler used in PropagateOneStep()
      G4VExceptionHandler* m_StepExceptionHandler;

      //! Pointer to the (un)cached magnetic field
      G4MagneticField* m_MagneticField;

      //! Pointer to the uncached magnetic field (might be superseded by its cached version)
      G4MagneticField* m_UncachedField;

      //! Pointer to the equation of motion in the magnetic field (if not the default)
      G4Mag_UsualEqRhs* m_MagFldEquation;

      //! Pointer to the equation-of-motion stepper (if not the default)
      G4MagIntegratorStepper* m_Stepper;

      //! Pointer to the equation-of-motion chord finder (if not the default)
      G4ChordFinder* m_ChordFinder;

      //! Pointer to the standard equation-of-motion stepper
      G4MagIntegratorStepper* m_StdStepper;

      //! Pointer to the forward-propagation equation of motion
      G4EquationOfMotion* m_ForwardEquationOfMotion;

      //! Pointer to the equation of motion that accommodates back-propagation
      G4EquationOfMotion* m_BackwardEquationOfMotion;

      //! Pointer to the visualization manager (if used)
      G4VisManager* m_VisManager;

    };

  } // end of namespace Simulation

} // end of namespace Belle2

#endif // EXTMANAGER_H
