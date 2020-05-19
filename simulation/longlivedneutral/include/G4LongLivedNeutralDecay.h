

#pragma once
#include "G4ios.hh"
#include "globals.hh"
#include "G4VRestDiscreteProcess.hh"
#include "G4ParticleChangeForDecay.hh"
#include "G4DecayProcessType.hh"
#include <framework/core/Module.h>

namespace Belle2 {


  class G4LongLivedNeutral;

  class G4LongLivedNeutralDecay : public G4VRestDiscreteProcess {
// Class Description
    //  This class is a decay process

  public:
    //  Constructors
    G4LongLivedNeutralDecay(const G4String& processName = "LongLivedNeutralDecay");

    //  Destructor
    virtual ~G4LongLivedNeutralDecay();

  private:
    //  copy constructor
    G4LongLivedNeutralDecay(const G4LongLivedNeutralDecay& right);

    //  Assignment Operation (generated)
    G4LongLivedNeutralDecay& operator=(const G4LongLivedNeutralDecay& right);

  public: //With Description
    // G4Decay Process has both
    // PostStepDoIt (for decay in flight)
    //   and
    // AtRestDoIt (for decay at rest)

    virtual G4VParticleChange* PostStepDoIt(
      const G4Track& aTrack,
      const G4Step& aStep
    ) override;

    virtual G4VParticleChange* AtRestDoIt(
      const G4Track& aTrack,
      const G4Step&  aStep
    ) override;

    virtual void BuildPhysicsTable(const G4ParticleDefinition&) override;
    // In G4Decay, thePhysicsTable stores values of
    //    beta * std::sqrt( 1 - beta*beta)
    //  as a function of normalized kinetic enregy (=Ekin/mass),
    //  becasuse this table is universal for all particle types,


    virtual G4bool IsApplicable(const G4ParticleDefinition&) override;
    // returns "true" if the decay process can be applied to
    // the particle type.

  protected: // With Description
    virtual G4VParticleChange* DecayIt(
      const G4Track& aTrack,
      const G4Step&  aStep
    );
    // The DecayIt() method returns by pointer a particle-change object,
    // which has information of daughter particles.

  public:
    virtual G4double AtRestGetPhysicalInteractionLength(
      const G4Track& track,
      G4ForceCondition* condition
    ) override;

    virtual G4double PostStepGetPhysicalInteractionLength(
      const G4Track& track,
      G4double   previousStepSize,
      G4ForceCondition* condition
    ) override;

  protected: // With Description
    // GetMeanFreePath returns ctau*beta*gamma for decay in flight
    // GetMeanLifeTime returns ctau for decay at rest
    virtual G4double GetMeanFreePath(const G4Track& aTrack,
                                     G4double   previousStepSize,
                                     G4ForceCondition* condition
                                    ) override;

    virtual G4double GetMeanLifeTime(const G4Track& aTrack,
                                     G4ForceCondition* condition
                                    ) override;

  public: //With Description
    virtual void StartTracking(G4Track*) override;
    virtual void EndTracking() override;
    // inform Start/End of tracking for each track to the physics process

  public: //With Description

    G4double GetRemainderLifeTime() const;
    //Get Remainder of life time at rest decay

  protected:
    G4int verboseLevel;
    // controle flag for output message
    //  0: Silent
    //  1: Warning message
    //  2: More

  protected:
    // HighestValue.
    const G4double HighestValue;

    // Remainder of life time at rest
    G4double fRemainderLifeTime;

    // ParticleChange for decay process
    G4ParticleChangeForDecay fParticleChangeForDecay;

  };

  inline
  G4VParticleChange* G4LongLivedNeutralDecay::AtRestDoIt(
    const G4Track& aTrack,
    const G4Step&  aStep
  )
  {
    return DecayIt(aTrack, aStep);
  }

  inline
  G4double G4LongLivedNeutralDecay::GetRemainderLifeTime() const
  {
    return fRemainderLifeTime;
  }

}







