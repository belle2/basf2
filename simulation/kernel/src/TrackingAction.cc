/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Andreas Moll                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/TrackingAction.h>
#include <simulation/kernel/UserInfo.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <G4TrackingManager.hh>
#include <G4Track.hh>

using namespace Belle2;
using namespace Belle2::Simulation;


TrackingAction::TrackingAction(MCParticleGraph& mcParticleGraph): G4UserTrackingAction(), m_mcParticleGraph(mcParticleGraph)
{

}


TrackingAction::~TrackingAction()
{

}


void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
  const G4DynamicParticle* dynamicParticle = track->GetDynamicParticle();

  try {
    //Check if the dynamic particle has a primary particle attached.
    //If yes, the UserInfo of the primary particle as UserInfo to the track.
    if (dynamicParticle->GetPrimaryParticle() != NULL) {
      const G4PrimaryParticle* primaryParticle = dynamicParticle->GetPrimaryParticle();
      if (primaryParticle->GetUserInformation() != NULL) {
        const_cast<G4Track*>(track)->SetUserInformation(new TrackInfo(ParticleInfo::getInfo(*primaryParticle)));
      } else {
        B2WARNING(track->GetDefinition()->GetPDGEncoding() << " has no MCParticle user information !")
      }
    }

    //Get particle of current track
    MCParticleGraph::GraphParticle& currParticle = TrackInfo::getInfo(*track);
    //Set the Values of the particle which are already known
    G4ThreeVector dpMom  = dynamicParticle->GetMomentum() * Unit::MeV;
    G4ThreeVector trVtxPos = track->GetVertexPosition() * Unit::mm;
    currParticle.setTrackID(track->GetTrackID());
    currParticle.setPDG(dynamicParticle->GetPDGcode());
    currParticle.setMass(dynamicParticle->GetMass() * Unit::MeV);
    currParticle.setEnergy(dynamicParticle->GetTotalEnergy() * Unit::MeV);
    currParticle.setMomentum(dpMom.x(), dpMom.y(), dpMom.z());
    currParticle.setProductionTime(track->GetGlobalTime() * Unit::ns);
    currParticle.setProductionVertex(trVtxPos.x(), trVtxPos.y(), trVtxPos.z());
  } catch (CouldNotFindUserInfo& exc) {
    B2FATAL(exc.what())
  }
}


void TrackingAction::PostUserTrackingAction(const G4Track* track)
{
  G4StepPoint* postStep = track->GetStep()->GetPostStepPoint();

  //Get particle of current track
  try {
    MCParticleGraph::GraphParticle& currParticle = TrackInfo::getInfo(*track);

    //Check if particle left detector.
    //fWorldBoundary seems to be broken, check if poststep is on boundary and next volume is 0
    if (postStep->GetStepStatus() == fGeomBoundary && track->GetNextVolume() == NULL) {
      currParticle.addStatus(MCParticle::c_LeftDetector);
    }

    //Check if particle was stopped in the detector
    if (track->GetKineticEnergy() <= 0.0) {
      currParticle.addStatus(MCParticle::c_StoppedInDetector);
    }

    //Set the values for the particle
    G4ThreeVector decVtx = postStep->GetPosition() * Unit::mm;
    currParticle.setDecayVertex(decVtx.x(), decVtx.y(), decVtx.z());
    currParticle.setDecayTime(postStep->GetGlobalTime() * Unit::ns);
    currParticle.setValidVertex(true);

    //Add particle and decay Information to all secondaries
    BOOST_FOREACH(G4Track* daughterTrack, *fpTrackingManager->GimmeSecondaries()) {

      //Add the particle to the particle graph and as UserInfo to the track
      //if it is a secondary particle created by Geant4.
      if (daughterTrack->GetDynamicParticle()->GetPrimaryParticle() == NULL) {
        MCParticleGraph::GraphParticle& graphParticle = m_mcParticleGraph.addParticle();
        const_cast<G4Track*>(daughterTrack)->SetUserInformation(new TrackInfo(graphParticle));

        currParticle.decaysInto(graphParticle); //Add the decay
      }
    }
  } catch (CouldNotFindUserInfo& exc) {
    B2FATAL(exc.what())
  }
}
