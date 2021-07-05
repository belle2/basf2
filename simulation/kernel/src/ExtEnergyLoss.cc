/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/kernel/ExtEnergyLoss.h>
#include <simulation/kernel/EnergyLossForExtrapolator.h>

#include <G4ErrorPropagatorData.hh>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

ExtEnergyLoss::ExtEnergyLoss(const G4String& processName, G4ProcessType type)
  : G4VContinuousProcess(processName, type), m_energyLossForExtrapolator(NULL)
{
  B2DEBUG(200, "ExtEnergyLoss is created");
  if (m_energyLossForExtrapolator == NULL) {
    m_energyLossForExtrapolator = new EnergyLossForExtrapolator;
  }
  m_StepLimit = 1.0;  // fraction of kinetic energy that could be lost in one step
  if (false) {
    G4Track aTrack;
    G4Step aStep;
    double dummy;
    AlongStepDoIt(aTrack, aStep);
    GetContinuousStepLimit(aTrack, 0.0, 0.0, dummy);
  }
}

ExtEnergyLoss::~ExtEnergyLoss()
{
  if (m_energyLossForExtrapolator != NULL) {
    delete m_energyLossForExtrapolator;
  }
}

G4VParticleChange* ExtEnergyLoss::AlongStepDoIt(const G4Track& aTrack, const G4Step& aStep)
{
  aParticleChange.Initialize(aTrack);

  G4ErrorPropagatorData* g4edata =  G4ErrorPropagatorData::GetErrorPropagatorData();

  G4double kinEnergyStart = aTrack.GetKineticEnergy();
  G4double step_length = aStep.GetStepLength();

  const G4Material* aMaterial = aTrack.GetMaterial();
  const G4ParticleDefinition* aParticleDef = aTrack.GetDynamicParticle()->GetDefinition();
  G4double kinEnergyEnd = kinEnergyStart;

  if (g4edata->GetMode() == G4ErrorMode(G4ErrorMode_PropBackwards)) {
    kinEnergyEnd = m_energyLossForExtrapolator->EnergyBeforeStep(kinEnergyStart,
                   step_length,
                   aMaterial,
                   aParticleDef);
    G4double kinEnergyHalfStep = (kinEnergyStart + kinEnergyEnd) * 0.5;

    B2DEBUG(200, "ExtEnergyLoss::AlongStepDoIt() BWD  end " << kinEnergyEnd << " halfstep " << kinEnergyHalfStep);

    // rescale to energy lost at midpoint of step
    kinEnergyEnd = m_energyLossForExtrapolator->EnergyBeforeStep(kinEnergyHalfStep,
                   step_length,
                   aMaterial,
                   aParticleDef);
    kinEnergyEnd = kinEnergyStart - (kinEnergyHalfStep - kinEnergyEnd);
  } else if (g4edata->GetMode() == G4ErrorMode(G4ErrorMode_PropForwards)) {

    kinEnergyEnd = m_energyLossForExtrapolator->EnergyAfterStep(kinEnergyStart,
                                                                step_length,
                                                                aMaterial,
                                                                aParticleDef);
    G4double kinEnergyHalfStep = (kinEnergyStart + kinEnergyEnd) * 0.5;
    B2DEBUG(200, "ExtEnergyLoss::AlongStepDoIt() FWD  end " << kinEnergyEnd << " halfstep " << kinEnergyHalfStep);

    // rescale to energy lost at midpoint of step
    kinEnergyEnd = m_energyLossForExtrapolator->EnergyAfterStep(kinEnergyHalfStep,
                                                                step_length,
                                                                aMaterial,
                                                                aParticleDef);
    kinEnergyEnd = kinEnergyStart - (kinEnergyHalfStep - kinEnergyEnd);
  }

  G4double edepo = kinEnergyEnd - kinEnergyStart;

  B2DEBUG(300, "ExtEnergyLoss::AlongStepDoIt() Estart= " << kinEnergyStart << " Eend " << kinEnergyEnd
          << " Ediff " << -edepo << " step= " << step_length << " mate= " << aMaterial->GetName()
          << " particle= " << aParticleDef->GetParticleName());

  aParticleChange.ClearDebugFlag();
  aParticleChange.ProposeLocalEnergyDeposit(edepo);
  aParticleChange.SetNumberOfSecondaries(0);

  aParticleChange.ProposeEnergy(kinEnergyEnd);

  return &aParticleChange;
}

G4double ExtEnergyLoss::GetContinuousStepLimit(const G4Track& aTrack,
                                               G4double,
                                               G4double currentMinimumStep,
                                               G4double&)
{
  G4double step = DBL_MAX;
  if (m_StepLimit < 1.0) {
    G4double kinEnergyStart = aTrack.GetKineticEnergy();
    G4double kinEnergyLoss = kinEnergyStart;
    const G4Material* aMaterial = aTrack.GetMaterial();
    const G4ParticleDefinition* aParticleDef = aTrack.GetDynamicParticle()->GetDefinition();
    G4ErrorPropagatorData* g4edata =  G4ErrorPropagatorData::GetErrorPropagatorData();
    if (g4edata->GetMode() == G4ErrorMode(G4ErrorMode_PropBackwards)) {
      kinEnergyLoss = - kinEnergyStart +
                      m_energyLossForExtrapolator->EnergyBeforeStep(kinEnergyStart, currentMinimumStep, aMaterial, aParticleDef);
    } else if (g4edata->GetMode() == G4ErrorMode(G4ErrorMode_PropForwards)) {
      kinEnergyLoss = kinEnergyStart -
                      m_energyLossForExtrapolator->EnergyAfterStep(kinEnergyStart, currentMinimumStep, aMaterial, aParticleDef);
    }
    B2DEBUG(300, "ExtEnergyLoss::GetContinuousStepLimit() currentMinimumStep " << currentMinimumStep
            << "  kinEnergyLoss " << kinEnergyLoss << " kinEnergyStart " << kinEnergyStart);
    if (kinEnergyLoss / kinEnergyStart > m_StepLimit) {
      step = m_StepLimit / (kinEnergyLoss / kinEnergyStart)  * currentMinimumStep;
      B2DEBUG(300, "ExtEnergyLoss::GetContinuousStepLimit() limiting Step " << step
              << " energy loss fraction " << kinEnergyLoss / kinEnergyStart << " > " << m_StepLimit);
    }
  }

  return step;

}

