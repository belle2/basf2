/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// modified from BelleII monopole simulation

#include <simulation/longlivedneutral/G4LongLivedNeutral.h>
#include <simulation/longlivedneutral/G4LongLivedNeutralDecay.h>

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4DynamicParticle.hh"
#include "G4PrimaryParticle.hh"
#include "G4DecayProducts.hh"
#include "G4DecayTable.hh"
#include "G4VDecayChannel.hh"
#include "G4PhysicsLogVector.hh"
#include "G4ParticleChangeForDecay.hh"


using namespace Belle2;

// constructor
G4LongLivedNeutralDecay::G4LongLivedNeutralDecay(const G4String& processName)
  : G4VRestDiscreteProcess(processName, fDecay),
    verboseLevel(1),
    HighestValue(20.0),
    fRemainderLifeTime(-1.0)

{
  // set Process Sub Type
  SetProcessSubType(static_cast<int>(DECAY));

#ifdef G4VERBOSE
  if (GetVerboseLevel() > 1) {
    G4cout << "G4LongLivedNeutralDecay  constructor " << "  Name:" << processName << G4endl;
  }
#endif

  pParticleChange = &fParticleChangeForDecay;
}

G4LongLivedNeutralDecay::~G4LongLivedNeutralDecay()
{

}

G4bool G4LongLivedNeutralDecay::IsApplicable(const G4ParticleDefinition& aParticleType)
{
  // original G4Decay checks for particle definition mass and lifetime, which are zero for generality
  // the correct mass and lifetime are set for the G4PrimaryParticle
  if (!aParticleType.GetPDGStable()) {
    return true;
  } else return false;
}


G4double G4LongLivedNeutralDecay::GetMeanLifeTime(const G4Track& aTrack  ,
                                                  G4ForceCondition*)
{
  const G4DynamicParticle* dynamicParticle = aTrack.GetDynamicParticle();
  const G4PrimaryParticle* primaryParticle = dynamicParticle->GetPrimaryParticle();
  G4double pTime = primaryParticle->GetProperTime();

  return  pTime;
}

G4double G4LongLivedNeutralDecay::GetMeanFreePath(const G4Track& aTrack, G4double, G4ForceCondition*)
{

  const G4DynamicParticle* dynamicParticle = aTrack.GetDynamicParticle();
  const G4PrimaryParticle* primaryParticle = dynamicParticle->GetPrimaryParticle();
  G4double pTime = primaryParticle->GetProperTime();
  G4double aMass = primaryParticle->GetMass();
  G4double aMomentum = primaryParticle->GetTotalMomentum();
  return c_light * pTime * aMomentum / aMass;
}

void G4LongLivedNeutralDecay::BuildPhysicsTable(const G4ParticleDefinition&)
{
  return;
}

G4VParticleChange* G4LongLivedNeutralDecay::DecayIt(const G4Track& aTrack, const G4Step&)
{
  // The DecayIt() method returns by pointer a particle-change object.
  // Units are expressed in GEANT4 internal units.
  // Initialize ParticleChange
  //   all members of G4VParticleChange are set to equal to
  //   corresponding member in G4Track
//

  fParticleChangeForDecay.Initialize(aTrack);

  // LongLivedNeutral: work only with DynamicParticle, not ParticleDefinition
  const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
  const G4PrimaryParticle* aPrimaryParticle = aParticle->GetPrimaryParticle();

  //check if thePreAssignedDecayProducts exists
  const G4DecayProducts* o_products = (aParticle->GetPreAssignedDecayProducts());
  G4bool isPreAssigned = (o_products != nullptr);
  G4DecayProducts* products = nullptr;

  if (isPreAssigned) {
    // default: copy decay products
    products = new G4DecayProducts(*o_products);
  } else {
    // Should not happen, since interaction and free path length are set to max
    G4ExceptionDescription ed;
    ed << "LongLivedNeutral particle:"
       << " decay probability exist but no pre-assigned decay products defined "
       << "- the particle will be killed;\n";
    G4Exception("G4LongLivedNeutralDecay::DecayIt ",
                "DECAY101", JustWarning, ed);

    fParticleChangeForDecay.SetNumberOfSecondaries(0);
    // Kill the parent particle
    fParticleChangeForDecay.ProposeTrackStatus(fStopAndKill) ;
    fParticleChangeForDecay.ProposeLocalEnergyDeposit(0.0);

    ClearNumberOfInteractionLengthLeft();
    return &fParticleChangeForDecay ;
  }

  // get parent particle information ...................................
  G4double   ParentEnergy  = aPrimaryParticle->GetTotalEnergy();
  G4double   ParentMass    = aPrimaryParticle->GetMass();
  if (ParentEnergy < ParentMass) {
    G4ExceptionDescription ed;
    ed << "Total Energy is less than its mass - increased the energy"
       << "\n Particle: " << "LongLivedNeutral"
       << "\n Energy:"    << ParentEnergy / MeV << "[MeV]"
       << "\n Mass:"      << ParentMass / MeV << "[MeV]";
    G4Exception("G4LongLivedNeutralDecay::DecayIt ",
                "DECAY102", JustWarning, ed);
    ParentEnergy = ParentMass;
  }

  G4ThreeVector ParentDirection(aPrimaryParticle->GetMomentumDirection());

  // assign a new parent to the products from info of the primary particle
  const G4DynamicParticle dParticle(aPrimaryParticle->GetParticleDefinition(),
                                    aPrimaryParticle->GetMomentumDirection(),
                                    aPrimaryParticle->GetKineticEnergy(),
                                    aPrimaryParticle->GetMass());

  products->SetParentParticle(dParticle);
  //boost all decay products to laboratory frame
  G4double energyDeposit = 0.0;
  G4double finalGlobalTime = aTrack.GetGlobalTime();
  G4double finalLocalTime = aTrack.GetLocalTime();
  if (aTrack.GetTrackStatus() == fStopButAlive) {
    // AtRest case
    finalGlobalTime += fRemainderLifeTime;
    finalLocalTime += fRemainderLifeTime;
    energyDeposit += aPrimaryParticle->GetKineticEnergy();
    products->Boost(ParentEnergy, ParentDirection);
  } else {
    // default for LongLivedNeutral: PostStep case (decay in flight)
    products->Boost(ParentEnergy, ParentDirection);
  }

  //add products in fParticleChangeForDecay
  G4int numberOfSecondaries = products->entries();
  fParticleChangeForDecay.SetNumberOfSecondaries(numberOfSecondaries);
#ifdef G4VERBOSE
  if (GetVerboseLevel() > 1) {
    G4cout << "G4LongLivedNeutralDecay::DoIt  : Decay vertex :";
    G4cout << " Time: " << finalGlobalTime / ns << "[ns]";
    G4cout << " proper time: " << finalLocalTime / ns << "[ns]";
    G4cout << " X:" << (aTrack.GetPosition()).x() / cm << "[cm]";
    G4cout << " Y:" << (aTrack.GetPosition()).y() / cm << "[cm]";
    G4cout << " Z:" << (aTrack.GetPosition()).z() / cm << "[cm]";
    G4cout << G4endl;
    G4cout << "G4LongLivedNeutralDecay::DoIt  : decay products in Lab. Frame" << G4endl;
    products->DumpInfo();
  }
#endif

  G4int index;
  G4ThreeVector currentPosition;
  const G4TouchableHandle thand = aTrack.GetTouchableHandle();
  for (index = 0; index < numberOfSecondaries; index++) {
    // get current position of the track
    currentPosition = aTrack.GetPosition();
    // create a new track object
    G4Track* secondary = new G4Track(products->PopProducts(),
                                     finalGlobalTime ,
                                     currentPosition);
    // switch on good for tracking flag
    secondary->SetGoodForTrackingFlag();
    secondary->SetTouchableHandle(thand);
    // add the secondary track in the List
    fParticleChangeForDecay.AddSecondary(secondary);
  }
  delete products;

  // Kill the parent particle
  fParticleChangeForDecay.ProposeTrackStatus(fStopAndKill) ;
  fParticleChangeForDecay.ProposeLocalEnergyDeposit(energyDeposit);
  fParticleChangeForDecay.ProposeLocalTime(finalLocalTime);

  // Clear NumberOfInteractionLengthLeft
  ClearNumberOfInteractionLengthLeft();

  return &fParticleChangeForDecay ;
}


void G4LongLivedNeutralDecay::StartTracking(G4Track*)
{
  currentInteractionLength = -1.0;
  ResetNumberOfInteractionLengthLeft();


  fRemainderLifeTime = -1.0;
}

void G4LongLivedNeutralDecay::EndTracking()
{
  // Clear NumberOfInteractionLengthLeft
  ClearNumberOfInteractionLengthLeft();

  currentInteractionLength = -1.0;
}


G4double G4LongLivedNeutralDecay::PostStepGetPhysicalInteractionLength(
  const G4Track& track,
  G4double   previousStepSize,
  G4ForceCondition* condition
)
{
  // condition is set to "Not Forced"
  *condition = NotForced;

  // pre-assigned Decay time
  G4double pTime = track.GetDynamicParticle()->GetPrimaryParticle()->GetProperTime();
  if (pTime >= 0.) {
    // default for LongLived Particle: pre-assigned decay time case
    fRemainderLifeTime = pTime - track.GetProperTime();
    if (fRemainderLifeTime <= 0.0) fRemainderLifeTime = 0.0;

    // use normalized kinetic energy (= Ekin/mass)
    G4double rvalue = c_light * fRemainderLifeTime;
    const G4DynamicParticle* dynamicParticle = track.GetDynamicParticle();
    const G4PrimaryParticle* primaryParticle = dynamicParticle->GetPrimaryParticle();
    G4double aMass = primaryParticle->GetMass();
    G4double aMomentum = primaryParticle->GetTotalMomentum();
    rvalue *= aMomentum / aMass;
    return rvalue;

  } else {
    // no pre-assigned proper decay time --> stable
    return previousStepSize * DBL_MAX;
  }
}

G4double G4LongLivedNeutralDecay::AtRestGetPhysicalInteractionLength(
  const G4Track& track,
  G4ForceCondition* condition
)
{
  // condition is set to "Not Forced"
  *condition = NotForced;
  G4double pTime = track.GetDynamicParticle()->GetPrimaryParticle()->GetProperTime();
  if (pTime >= 0.) {
    // general case for LongLivedNeutral
    fRemainderLifeTime = pTime - track.GetProperTime();
    if (fRemainderLifeTime <= 0.0) fRemainderLifeTime = DBL_MIN;
  } else {
    // LongLivedNeutral case: set stable if there is no pre-assigned decay time
    fRemainderLifeTime = 1e24 * s;
  }
  return fRemainderLifeTime;
}


G4VParticleChange* G4LongLivedNeutralDecay::PostStepDoIt(
  const G4Track& aTrack,
  const G4Step&  aStep
)
{
  if ((aTrack.GetTrackStatus() == fStopButAlive) ||
      (aTrack.GetTrackStatus() == fStopAndKill)) {
    fParticleChangeForDecay.Initialize(aTrack);
    return &fParticleChangeForDecay;
  } else {
    return DecayIt(aTrack, aStep);
  }
}

