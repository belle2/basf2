/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#pragma once

// inline include at the end of the file

#include <G4VProcess.hh>
#include <G4FieldManager.hh>
#include <G4Navigator.hh>
#include <G4TransportationManager.hh>
#include <G4PropagatorInField.hh>
#include <G4Track.hh>
#include <G4Step.hh>
#include <G4ParticleChangeForTransport.hh>

#include <simulation/monopoles/G4MonopoleFieldSetup.h>

class G4SafetyHelper;

namespace Belle2 {

  namespace Monopoles {

    class G4Monopole;

    /**
     * Concrete class that does the geometrical transport.
     *
     * Quite big and clunky class of the monopole transportation.
     *
     */
    class G4MonopoleTransportation : public G4VProcess {

    public:

      /**
       * Constructor.
       * @param mpl Reference to the monopole definition in GEANT4
       * @param verb Verbosity level, if GEANT4 was set to be verbose
       */
      G4MonopoleTransportation(const G4Monopole* mpl, G4int verb = 1);
      /**
       * Destructor.
       */
      ~G4MonopoleTransportation();

      /**
       * G4VProcess::AlongStepGetPhysicalInteractionLength() implementation.
       *
       * Responsibilities:
       * Find whether the geometry limits the Step, and to what length
       * Calculate the new value of the safety and return it.
       * Store the final time, position and momentum.
       *
       * @param track Propagating particle track reference
       * @param previousStepSize This argument of base function is ignored
       * @param currentMinimumStep Current minimum step size
       * @param currentSafety Reference to current step safety
       * @param selection Pointer for return value of GPILSelection, which is set to default value of CandidateForSelection
       * @return Next geometry step length
       */
      virtual G4double AlongStepGetPhysicalInteractionLength(
        const G4Track& track,
        G4double  previousStepSize,
        G4double  currentMinimumStep,
        G4double& currentSafety,
        G4GPILSelection* selection
      );

      /**
       * G4VProcess::AlongStepDoIt() implementation,
       * Proposes changes during step to fParticleChange of this class.
       *
       * @param track Propagating particle track reference
       * @param stepData Current step reference
       * @return Reference to modified fParticleChange
       */
      virtual G4VParticleChange* AlongStepDoIt(
        const G4Track& track,
        const G4Step& stepData
      );

      /**
       * G4VProcess::PostStepDoIt() implementation.
       * Proposes changes after the step to fParticleChange of this class.
       *
       * Responsible for the relocation
       * @param track Propagating particle track reference
       * @param stepData Current step reference
       * @return Reference to modified fParticleChange
       */
      virtual G4VParticleChange* PostStepDoIt(
        const G4Track& track,
        const G4Step&  stepData
      );

      /**
       * G4VProcess::PostStepGetPhysicalInteractionLength() implementation.
       * Forces the PostStepDoIt action to be called,
       * so that it can do the relocation if it is needed,
       * but does not limit the step.
       *
       * @param track This argument of base function is ignored
       * @param previousStepSize This argument of base function is ignored
       * @param pForceCond Force condition by default
       * @return DBL_MAX
       */
      virtual G4double PostStepGetPhysicalInteractionLength(
        const G4Track& track,
        G4double   previousStepSize,
        G4ForceCondition* pForceCond
      );

      inline G4PropagatorInField* GetPropagatorInField(); /**< Access fFieldPropagator, the assistant class that Propagate in a Field.*/
      inline void SetPropagatorInField(G4PropagatorInField*
                                       pFieldPropagator); /**< Set fFieldPropagator, the assistant class that Propagate in a Field.*/

      inline G4double GetThresholdWarningEnergy() const; /**< Access fThreshold_Warning_Energy*/
      inline G4double GetThresholdImportantEnergy() const; /**< Access fThreshold_Important_Energy*/
      inline G4int GetThresholdTrials() const; /**< Access fThresholdTrials*/

      inline void SetThresholdWarningEnergy(G4double newEnWarn); /**< Set fThreshold_Warning_Energy*/
      inline void SetThresholdImportantEnergy(G4double newEnImp); /**< Set fThreshold_Important_Energy*/
      inline void SetThresholdTrials(G4int newMaxTrials); /**< Set fThresholdTrials*/

      // Get/Set parameters for killing loopers:
      //   Above 'important' energy a 'looping' particle in field will
      //   *NOT* be abandoned, except after fThresholdTrials attempts.
      // Below Warning energy, no verbosity for looping particles is issued

      inline G4double GetMaxEnergyKilled() const; /**< Access fMaxEnergyKilled*/
      inline G4double GetSumEnergyKilled() const; /**< Access fSumEnergyKilled*/
      inline void ResetKilledStatistics(G4int report = 1); /**< Statistics for tracks killed (currently due to looping in field)*/

      inline void EnableShortStepOptimisation(G4bool optimise =
                                                true); /**< Whether short steps < safety will avoid to call Navigator (if field=0) */

    public:

      /**
       * No operation in  AtRestDoIt.
       */
      virtual G4double AtRestGetPhysicalInteractionLength(
        const G4Track&,
        G4ForceCondition*
      ) { return -1.0; };

      /**
       * No operation in  AtRestDoIt.
       */
      virtual G4VParticleChange* AtRestDoIt(
        const G4Track&,
        const G4Step&
      ) {return 0;};

      virtual void StartTracking(G4Track* aTrack); /**< Reset state for new (potentially resumed) track*/

    protected:

      G4bool               DoesGlobalFieldExist(); /**< Checks whether a field exists for the "global" field manager*/

    private:

      const G4Monopole* fParticleDef; /**< Monopole definition for charge and mass reference*/

      G4MonopoleFieldSetup*  fMagSetup; /**< Monpole field setup*/

      G4Navigator*         fLinearNavigator; /**< Propagator used to transport the particle*/
      G4PropagatorInField* fFieldPropagator; /**< Propagator used to transport the particle*/

      G4ThreeVector        fTransportEndPosition; /**< The particle's state after this Step, Store for DoIt*/
      G4ThreeVector        fTransportEndMomentumDir; /**< The particle's state after this Step, Store for DoIt*/
      G4double             fTransportEndKineticEnergy; /**< The particle's state after this Step, Store for DoIt*/
      G4ThreeVector        fTransportEndSpin; /**< The particle's state after this Step, Store for DoIt*/
      G4bool               fMomentumChanged; /**< The particle's state after this Step, Store for DoIt*/
      //  G4bool               fEnergyChanged; /**< The particle's state after this Step, Store for DoIt*/
      G4bool               fEndGlobalTimeComputed; /**< The particle's state after this Step, Store for DoIt*/
      G4double             fCandidateEndGlobalTime; /**< The particle's state after this Step, Store for DoIt*/

      G4bool               fParticleIsLooping; /**< Is the monopole stuck in looping*/

      G4TouchableHandle    fCurrentTouchableHandle; /**< Current touchable handle*/

      G4bool fGeometryLimitedStep; /**< Flag to determine whether a boundary was reached*/

      G4ThreeVector  fPreviousSftOrigin; /**< Remember last safety origin*/
      G4double       fPreviousSafety; /**< Remember last safety value*/

      G4ParticleChangeForTransport fParticleChange; /**< New ParticleChange*/

      G4double endpointDistance; /**< Endpint distance */

      G4double fThreshold_Trap_Energy; /**< Assume monopoles below this can bound to material*/ //FIXME should be dependent on e.g. density

      // Thresholds for looping particles:
      G4double fThreshold_Warning_Energy; /**< Warn above this energy about looping particle*/
      G4double fThreshold_Important_Energy; /**< Hesitate above this about looping particle for a certain no of trials*/
      G4int    fThresholdTrials; /**< Nubmer of trials for looping particles*/
      // Above 'important' energy a 'looping' particle in field will
      //   *NOT* be abandoned, except after fThresholdTrials attempts.
      // G4double fUnimportant_Energy;
      //  Below this energy, no verbosity for looping particles is issued

      G4int    fNoLooperTrials; /**< Counter for steps in which particle reports 'looping', if it is above 'Important' Energy*/
      G4double fSumEnergyKilled; /**< Sum of abandoned looping tracks energies*/
      G4double fMaxEnergyKilled; /**< Max of abandoned looping tracks energies*/

      /**
       * Whether to avoid calling G4Navigator for short step ( < safety)
       * If using it, the safety estimate for endpoint will likely be smaller.
       */
      G4bool   fShortStepOptimisation;

      G4SafetyHelper* fpSafetyHelper; /**< To pass it the safety value obtained*/

    };

  } //end Monopoles namespace

} //end Belle2 namespace

#include <simulation/monopoles/G4MonopoleTransportationInline.h>
