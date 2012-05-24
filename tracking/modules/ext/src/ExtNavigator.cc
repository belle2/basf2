/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Leo Piilonen                                            *
 * Derived from:  G4ErrorPropagationNavigator.cc                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/ext/ExtNavigator.h>
#include <globals.hh>
#include <G4ErrorPropagatorData.hh>
#include <G4ErrorSurfaceTarget.hh>

#include <framework/logging/Logger.h>

using namespace Belle2;

ExtNavigator::ExtNavigator() : G4Navigator()
{
}

ExtNavigator::~ExtNavigator()
{
}

G4double ExtNavigator::ComputeStep(const G4ThreeVector& pGlobalPoint,
                                   const G4ThreeVector& pDirection,
                                   const G4double pCurrentProposedStepLength,
                                   G4double& pNewSafety)
{
  G4double step = G4Navigator::ComputeStep(pGlobalPoint, pDirection,
                                           pCurrentProposedStepLength,
                                           pNewSafety);

  G4ErrorPropagatorData* g4edata = G4ErrorPropagatorData::GetErrorPropagatorData();

  // Examine target only if ext is active; ignore it otherwise.
  // (g4edata's state will be G4ErrorState_Idle between propagations)
  if ((g4edata != 0) && (g4edata->GetState() == G4ErrorState_Propagating)) {
    const G4ErrorTarget* target = g4edata->GetTarget();
    if (target != 0) {
      G4double targetSafety = target->GetDistanceFromPoint(pGlobalPoint);
      G4double stepPlane = target->GetDistanceFromPoint(pGlobalPoint, pDirection);
      if (stepPlane < 0.0) { // Negative means target is crossed, will not be found
        stepPlane = DBL_MAX;
      }
      B2DEBUG(300, "Module ext: ExtNavigator::ComputeStep(): target step "
              << stepPlane << " transportation step " << step)

      if (stepPlane < pCurrentProposedStepLength) {
        if (stepPlane < step) {
          B2DEBUG(300, "Module ext: ExtNavigator::ComputeStep(): target is closer than boundary. "
                  << "target step " << stepPlane << " transportation step " << step)
          step = stepPlane;
          g4edata->SetState(G4ErrorState_TargetCloserThanBoundary);
        }
      }
      // Not necessary to recompute detector-safety nor target-safety every time!
      // Detector-safety is already up-to-date; target-safety is updated above.
      //    pNewSafety = ComputeSafety(pGlobalPoint, pCurrentProposedStepLength);
      if (targetSafety < pNewSafety) {
        pNewSafety = targetSafety;
      }
      B2DEBUG(300, "Module ext: ExtNavigator::ComputeStep(): step " << step << " safety " << pNewSafety)
    }

  }
  return step;
}

G4double ExtNavigator::ComputeSafety(const G4ThreeVector& pGlobalPoint,
                                     const G4double pMaxLength)
{
  G4double newSafety = G4Navigator::ComputeSafety(pGlobalPoint, pMaxLength);

  G4ErrorPropagatorData* g4edata = G4ErrorPropagatorData::GetErrorPropagatorData();

  // Examine target only if ext is active; ignore it otherwise.
  // (g4edata's state will be G4ErrorState_Idle between propagations)
  if ((g4edata != 0) && (g4edata->GetState() == G4ErrorState_Propagating)) {

    const G4ErrorTarget* target = g4edata->GetTarget();
    if (target != 0) {
      G4double distance = target->GetDistanceFromPoint(pGlobalPoint);
      if (distance < newSafety) {
        newSafety = distance;
      }
    }
  }
  return newSafety;
}
