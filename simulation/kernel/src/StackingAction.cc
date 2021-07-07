/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/kernel/StackingAction.h>
#include <simulation/kernel/UserInfo.h>

#include <G4ParticleDefinition.hh>
#include <G4ParticleTypes.hh>
#include <G4Track.hh>
#include <G4VProcess.hh>

#include <TRandom.h>

using namespace std;
using namespace Belle2;
using namespace Simulation;


StackingAction::StackingAction(): m_photonFraction(1.0)
{
  if (false) {
    G4Track* aTrack;
    ClassifyNewTrack(aTrack);
    NewStage();
    PrepareNewEvent();
  }
}

StackingAction::~StackingAction()
{

}


G4ClassificationOfNewTrack StackingAction::ClassifyNewTrack(const G4Track* aTrack)
{
  // look for optical photon
  if (aTrack->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition())
    return fUrgent;

  // check if creator process is available
  if (!aTrack->GetCreatorProcess()) return fUrgent;

  // look for Cerenkov photon
  if (aTrack->GetCreatorProcess()->GetProcessName() != "Cerenkov") return fUrgent;

  // get track info
  TrackInfo* info = dynamic_cast<TrackInfo*>(aTrack->GetUserInformation());
  if (!info) return fUrgent;

  // chech if prescaling already done
  if (info->getStatus() != 0) return fUrgent;

  // if not, do it
  if (gRandom->Uniform() > m_photonFraction) {
    TrackInfo::getInfo(*aTrack).setIgnore();
    return fKill;
  }

  // set new status and store prescaling fraction
  info->setStatus(1);
  info->setFraction(m_photonFraction);

  return fUrgent;
}


void StackingAction::NewStage()
{

}


void StackingAction::PrepareNewEvent()
{

}
