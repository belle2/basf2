/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sascha Dreyer                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// modified from GEANT4 G4Transportation class

#pragma once
#include "G4ios.hh"
#include "globals.hh"
#include "G4VRestDiscreteProcess.hh"
#include "G4ParticleChangeForDecay.hh"
#include "G4DecayProcessType.hh"
#include <framework/core/Module.h>

namespace Belle2 {


  class G4LongLivedNeutral;
  /**
   * This class is a decay process
   */
  class G4LongLivedNeutralDecay : public G4VRestDiscreteProcess {

  public:
    /**
     * Constructor
     * @param processName decay process name
     */
    G4LongLivedNeutralDecay(const G4String& processName = "LongLivedNeutralDecay");

    /**
     * Destructor
     */
    virtual ~G4LongLivedNeutralDecay();

  private:
    /**
     * Copy Constructor
     * @param right copy reference
     */
    G4LongLivedNeutralDecay(const G4LongLivedNeutralDecay& right);

    /**
      * Assignment Operator
      * @param right assign reference
     */
    G4LongLivedNeutralDecay& operator=(const G4LongLivedNeutralDecay& right);

  public:

    /**
     * G4VProcess::PostStepDoIt() implemention
     * @param aTrack
     * @param aStep
     *
     * for decay in flight
     */
    virtual G4VParticleChange* PostStepDoIt(
      const G4Track& aTrack,
      const G4Step& aStep
    ) override;

    /**
     * G4VProess::AtRestDoIt() implemention for decay at rest
     * @param aTrack
     * @param aStep
     */
    virtual G4VParticleChange* AtRestDoIt(
      const G4Track& aTrack,
      const G4Step&  aStep
    ) override;

    virtual void BuildPhysicsTable(const G4ParticleDefinition&) override;
    /**< In G4Decay, thePhysicsTable stores values of
         beta * std::sqrt( 1 - beta*beta)
         as a function of normalized kinetic enregy (=Ekin/mass),
         becasuse this table is universal for all particle types*/


    virtual G4bool IsApplicable(const G4ParticleDefinition&) override;
    /**< returns "true" if the decay process can be applied to
         the particle type. */

  protected:

    /**
     * G4Decay::DecayIt() implemention
     * @param aTrack
     * @param aStep
     *
     * The DecayIt() method returns by pointer a particle-change object,
     * which has information of daughter particles.
     */
    virtual G4VParticleChange* DecayIt(
      const G4Track& aTrack,
      const G4Step&  aStep
    );

  public:

    /**
     * G4VProcess::AtRestGetPhysicalInteractionLength() implemention
     * @param track
     * @param condition
     *
     * No operation in AtRestDoIt.
     */
    virtual G4double AtRestGetPhysicalInteractionLength(
      const G4Track& track,
      G4ForceCondition* condition
    ) override;

    /**
     * G4VProcess::PostStepGetPhysicalInteractionLength() implemention
     * @param track This argument of base function is ignored
     * @param previousStepSize This argument of base function is ignored
     * @param condition
     *
     * Forces the PostStepDoIt action to be called, so that it can do the relocation if it is needed,
     * but does not limit the step.
     */
    virtual G4double PostStepGetPhysicalInteractionLength(
      const G4Track& track,
      G4double   previousStepSize,
      G4ForceCondition* condition
    ) override;

  protected:


    virtual G4double GetMeanFreePath(const G4Track& aTrack,
                                     G4double   previousStepSize,
                                     G4ForceCondition* condition
                                    ) override;
    /**< GetMeanFreePath returns ctau*beta*gamma for decay in flight*/

    virtual G4double GetMeanLifeTime(const G4Track& aTrack,
                                     G4ForceCondition* condition
                                    ) override;
    /**<  GetMeanLifeTime returns ctau for decay at rest*/



  public:

    virtual void StartTracking(G4Track*) override;
    /**< inform Start of tracking for each track to the physics process*/
    virtual void EndTracking() override;
    /**< inform End of tracking for each track to the physics process */


  public:

    G4double GetRemainderLifeTime() const;
    /**< Get Remainder of life time at rest decay*/

  protected:
    G4int verboseLevel;
    /**< controle flag for output message
     *   0: Silent
     *   1: Warning message
     *   2: More */

  protected:

    const G4double HighestValue; /**< Remainder of life time at rest */

    G4double fRemainderLifeTime; /**< ParticleChange for decay process */

    G4ParticleChangeForDecay fParticleChangeForDecay; /**< ParticleChange for decay process */

  };

  /**
   * G4VProcess::AtRestDoIt() implementation,
   * Proposes particle change by DecayIt method at rest
   *
   * @param aTrack Propagating particle track reference
   * @param aStep Current step reference
   */
  inline
  G4VParticleChange* G4LongLivedNeutralDecay::AtRestDoIt(
    const G4Track& aTrack,
    const G4Step&   aStep
  )
  {
    return DecayIt(aTrack, aStep);
  }


  inline
  G4double G4LongLivedNeutralDecay::GetRemainderLifeTime() const
  {
    return fRemainderLifeTime;
  }/**< Returns remaining lifetime */

}







