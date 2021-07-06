/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// modified from GEANT4 G4Transportation class


#ifndef G4LongLivedNeutralTransportation_hh
#define G4LongLivedNeutralTransportation_hh 1

#include "G4VProcess.hh"
#include "G4FieldManager.hh"

#include "G4Navigator.hh"
#include "G4TransportationManager.hh"
#include "G4PropagatorInField.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4ParticleChangeForTransport.hh"

class G4SafetyHelper;
class G4CoupledTransportation;
namespace Belle2 {

  /**
   * Concrete class that does the geometrical transport
   */
  class G4LongLivedNeutralTransportation : public G4VProcess {

  public:

    /**
     * Constructor
     * @param  verbosityLevel
     */
    G4LongLivedNeutralTransportation(G4int verbosityLevel = 1);

    /**
     * Destructor
     */
    ~G4LongLivedNeutralTransportation();

    /**
     * G4VProcess::AlongStepGetPhysicalInteractionLength() implementation,
     *
     * @param track Propagating particle track reference
     * @param previousStepSize This argument of base function is ignored
     * @param currentMinimumStep Current minimum step size
     * @param currentSafety Reference to current step safety
     * @param selection Pointer for return value of GPILSelection, which is set to default value of CandidateForSelection
     * @return Next geometry step length
     */
    G4double AlongStepGetPhysicalInteractionLength(
      const G4Track& track,
      G4double  previousStepSize,
      G4double  currentMinimumStep,
      G4double& currentSafety,
      G4GPILSelection* selection
    );

    /**
     * G4VProcess::AlongStepDoIt() implementation,
     *
     * @param track Propagating particle track reference
     * @param stepData Current step reference
     */
    G4VParticleChange* AlongStepDoIt(
      const G4Track& track,
      const G4Step& stepData
    );

    /**
     * G4VProcess::PostStepDoIt() implementation,
     *
     * @param track
     * @param stepData
     */
    G4VParticleChange* PostStepDoIt(
      const G4Track& track,
      const G4Step&  stepData
    );

    /**
     * G4VProcess::PostStepGetPhysicalInteractionLength() implementation.
     *
     * @param track This argument of base function is ignored
     * @param previousStepSize This argument of base function is ignored
     * @param pForceCond Force condition by default
     *
     * Forces the PostStepDoIt action to be called,
     * but does not limit the step
     */
    G4double PostStepGetPhysicalInteractionLength(
      const G4Track&,
      G4double   previousStepSize,
      G4ForceCondition* pForceCond
    );



    inline G4bool FieldExertedForce() { return fFieldExertedForce; }
    /**< References fFieldExertedForce.*/

    G4PropagatorInField* GetPropagatorInField();
    /**< Access fFieldPropagator, the assistant class that Propagate in a Field.*/
    void SetPropagatorInField(G4PropagatorInField* pFieldPropagator);
    /**< Access/set the assistant class that Propagate in a Field */

    inline G4double GetThresholdWarningEnergy() const; /**< Access fThreshold_Warning_Energy*/
    inline G4double GetThresholdImportantEnergy() const; /**< Access fThreshold_Important_Energy*/
    inline G4int GetThresholdTrials() const; /**< Access fThresholdTrials*/



    inline void SetThresholdWarningEnergy(G4double newEnWarn)
    {
      fThreshold_Warning_Energy = newEnWarn;
    } /**< Set fThreshold_Warning_Energy*/
    inline void SetThresholdImportantEnergy(G4double newEnImp)
    {
      fThreshold_Important_Energy = newEnImp;
    } /**< Set fThreshold_Important_Energy*/
    inline void SetThresholdTrials(G4int newMaxTrials)
    {
      fThresholdTrials = newMaxTrials;
    }/**< Set fThresholdTrials*/


    /** Get/Set parameters for killing loopers:
     *  Above 'important' energy a 'looping' particle in field will
     *  *NOT* be abandoned, except after fThresholdTrials attempts.
     *  Below Warning energy, no verbosity for looping particles is issued
     */

    void SetHighLooperThresholds();
    /** Shortcut method - old values (meant for HEP)*/
    void SetLowLooperThresholds();
    /** Set low thresholds - for low-E applications */
    void ReportLooperThresholds();
    /** Print values of looper thresholds */

    inline G4double GetMaxEnergyKilled() const;
    /**< Access fMaxEnergyKilled*/
    inline G4double GetSumEnergyKilled() const;
    /**< Access fSumEnergyKilled*/
    inline void ResetKilledStatistics(G4int report = 1);
    /**< Statistics for tracks killed (currently due to looping in field) */

    inline void EnableShortStepOptimisation(G4bool optimise = true);
    /**< Whether short steps < safety will avoid to call Navigator (if field=0)*/

    static void   SetSilenceLooperWarnings(G4bool val);
    /**< Do not warn about 'looping' particles */
    static G4bool GetSilenceLooperWarnings();
    /**< Do not throw exception about 'looping' particles */

  public:  // without description

    /**
     * No operation in  AtRestGPIL.
     */
    G4double AtRestGetPhysicalInteractionLength(const G4Track&,
                                                G4ForceCondition*)
    { return -1.0; }

    /**
     * No operation in  AtRestDoIt.
     */
    G4VParticleChange* AtRestDoIt(const G4Track&, const G4Step&)
    { return 0; }

    void StartTracking(G4Track* aTrack);
    /**< Reset state for new (potentially resumed) track*/

    /**
     * G4LongLivedNeutralTransportation::ProcessDescription()
     *
     * @outfile Description of process
     */
    virtual void ProcessDescription(std::ostream& outFile) const;

    void PrintStatistics(std::ostream& outStr) const;
    /**< returns current logging info of the algorithm */

  protected:

    /**
     * Checks whether a field exists for the "global" field manager.
     */
    inline G4bool DoesGlobalFieldExist()
    {
      G4TransportationManager* transportMgr;
      transportMgr = G4TransportationManager::GetTransportationManager();

      // fFieldExists= transportMgr->GetFieldManager()->DoesFieldExist();
      // return fFieldExists;
      return transportMgr->GetFieldManager()->DoesFieldExist();
    }

  private:

    G4Navigator* fLinearNavigator; /**< Propagator used to transport the particle*/
    G4PropagatorInField* fFieldPropagator; /**< Propagator used to transport the particle*/

    G4ThreeVector fTransportEndPosition = G4ThreeVector(0.0, 0.0, 0.0); /**< The particle's state after this Step, Store for DoIt*/
    G4ThreeVector fTransportEndMomentumDir = G4ThreeVector(0.0, 0.0, 0.0); /**< The particle's state after this Step, Store for DoIt*/
    G4double      fTransportEndKineticEnergy = 0.0; /**< The particle's state after this Step, Store for DoIt*/
    G4ThreeVector fTransportEndSpin = G4ThreeVector(0.0, 0.0, 0.0); /**< The particle's state after this Step, Store for DoIt*/
    G4bool        fMomentumChanged =  true; /**< The particle's state after this Step, Store for DoIt*/
    G4bool        fEndGlobalTimeComputed = false; /**< The particle's state after this Step, Store for DoIt*/
    G4double      fCandidateEndGlobalTime = 0.0; /**< The particle's state after this Step, Store for DoIt*/

    G4bool        fAnyFieldExists = false; /**< Flag for existing fields*/

    G4bool fNewTrack = true;          /**< Flag from StartTracking*/
    G4bool fFirstStepInVolume = true; /**< Flag first step in a geom. volume*/
    G4bool fLastStepInVolume = false; /**< Flag last step in a geom. volume*/
    G4bool fGeometryLimitedStep = true; /**< Flag to determine whether a boundary was reached*/

    G4bool fFieldExertedForce = false; /**< During current step*/

    G4TouchableHandle fCurrentTouchableHandle; /**< Current touchable handle*/

    G4ThreeVector fPreviousSftOrigin; /**< Remember last safety origin*/
    G4double      fPreviousSafety; /**< Remember last safety value*/

    G4ParticleChangeForTransport fParticleChange; /**< New ParticleChange*/

    G4double fEndPointDistance; /**< Endpoint distance */

    // Thresholds for looping particles:
    //
    G4double fThreshold_Warning_Energy =   1.0 * CLHEP::keV;  /**< Warn above this energy about looping particle*/
    G4double fThreshold_Important_Energy = 1.0 * CLHEP::MeV;  /**< Give a few trial above this E for looping particle*/
    G4int    fThresholdTrials = 10;  /**< Number of trials an important looper survives*/
    // Above 'important' energy a 'looping' particle in field will
    // *NOT* be abandoned, except after fThresholdTrials attempts.
    G4int    fAbandonUnstableTrials = 0; /**<  Number of trials after which to abandon*/
    //   unstable loopers ( 0 = never )
    // Counter for steps in which particle reports 'looping',
    //  ( Used if it is above 'Important' Energy. )
    G4int    fNoLooperTrials = 0; /**< Counter for steps in which particle reports 'looping', if it is above 'Important' Energy*/

    // Statistics for tracks abandoned due to looping, only used as flag, no looping neutral particles
    //
    G4double fSumEnergyKilled = 0.0;/**< Sum of abandoned looping tracks energies*/
    /**
     * Whether to avoid calling G4Navigator for short step ( < safety)
     * If using it, the safety estimate for endpoint will likely be smaller.
     */
    G4bool   fShortStepOptimisation;

    G4SafetyHelper* fpSafetyHelper; /**< To pass it the safety value obtained*/

  private:

    friend class G4CoupledTransportation;
    static G4bool fUseMagneticMoment; /**< Flag take into account magnetic moment*/
    static G4bool fUseGravity; /**< Flag take into account gravity*/
    static G4bool fSilenceLooperWarnings;  /**< Flag to *Supress* all 'looper' warnings*/

  };
}

#endif
