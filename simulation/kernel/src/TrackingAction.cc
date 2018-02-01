/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Andreas Moll, Marko Staric                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/TrackingAction.h>
#include <simulation/kernel/UserInfo.h>
#include <simulation/kernel/SensitiveDetectorBase.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <G4TrackingManager.hh>
#include <G4Track.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTypes.hh>
#include <G4EmProcessSubType.hh>
#include <G4DecayProcessType.hh>
#include <G4Event.hh>

using namespace Belle2;
using namespace Belle2::Simulation;

TrackingAction::TrackingAction(MCParticleGraph& mcParticleGraph):
  G4UserTrackingAction(), m_mcParticleGraph(mcParticleGraph),
  m_ignoreOpticalPhotons(false),
  m_ignoreSecondaries(false), m_secondariesEnergyCut(0.0),
  m_ignoreBremsstrahlungPhotons(false), m_bremsstrahlungPhotonsEnergyCut(0.0),
  m_ignorePairConversions(false), m_pairConversionsEnergyCut(0.0),
  m_storeTrajectories(false), m_distanceTolerance(0),
  m_storeMCTrajectories(), m_relMCTrajectories(StoreArray<MCParticle>(), m_storeMCTrajectories)
{
  if (false) {
    G4Track* aTrack;
    PreUserTrackingAction(aTrack);
    PostUserTrackingAction(aTrack);
  }
}


TrackingAction::~TrackingAction()
{

}

void TrackingAction::setStoreTrajectories(int store, double distanceTolerance)
{
  m_storeTrajectories = store;
  m_distanceTolerance = distanceTolerance;
  if (store) {
    // registration of store arrays and relations
    m_storeMCTrajectories.registerInDataStore();
    StoreArray<MCParticle> mcParticles;
    mcParticles.registerRelationTo(m_storeMCTrajectories);

    // additional registration of MCParticle relation
    // (note: m_relMCTrajectories is already defined by TrackingAction::TrackingAction)
    SensitiveDetectorBase::registerMCParticleRelation(m_relMCTrajectories, RelationArray::c_negativeWeight);
  }
}

void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
  //We only want to do the following for new tracks, not for suspended and reactivated ones"
  if (track->GetCurrentStepNumber() > 0) return;

  const G4DynamicParticle* dynamicParticle = track->GetDynamicParticle();

  try {
    //Check if the dynamic particle has a primary particle attached.
    //If the answer is yes, the UserInfo of the primary particle is recycled as the UserInfo of the track.
    if (dynamicParticle->GetPrimaryParticle() != NULL) {
      const G4PrimaryParticle* primaryParticle = dynamicParticle->GetPrimaryParticle();
      if (primaryParticle->GetUserInformation() != NULL) {
        const_cast<G4Track*>(track)->SetUserInformation(new TrackInfo(ParticleInfo::getInfo(*primaryParticle)));
      } else {
        B2WARNING(track->GetDefinition()->GetPDGEncoding() << " has no MCParticle user information !");
      }
    }

    //Get particle of current track
    MCParticleGraph::GraphParticle& currParticle = TrackInfo::getInfo(*track);
    //Set the Values of the particle which are already known
    G4ThreeVector dpMom  = dynamicParticle->GetMomentum() / CLHEP::MeV * Unit::MeV;
    G4ThreeVector trVtxPos = track->GetVertexPosition() / CLHEP::mm * Unit::mm;
    currParticle.setTrackID(track->GetTrackID());
    currParticle.setPDG(dynamicParticle->GetPDGcode());
    currParticle.setMass(dynamicParticle->GetMass() / CLHEP::MeV * Unit::MeV);
    currParticle.setEnergy(dynamicParticle->GetTotalEnergy() / CLHEP::MeV * Unit::MeV);
    currParticle.setMomentum(dpMom.x(), dpMom.y(), dpMom.z());
    currParticle.setProductionTime(track->GetGlobalTime()); // Time does not need a conversion factor
    currParticle.setProductionVertex(trVtxPos.x(), trVtxPos.y(), trVtxPos.z());

    //Primary or secondary particle?
    if (dynamicParticle->GetPrimaryParticle() != NULL) {
      //Primary particle
      currParticle.setSecondaryPhysicsProcess(0);
      currParticle.setIgnore(false);  //Store the generator info in the MCParticles block.

    } else if (track->GetCreatorProcess() != NULL) {
      //Secondary particle
      const int& processSubType = track->GetCreatorProcess()->GetProcessSubType();
      currParticle.setSecondaryPhysicsProcess(processSubType); //Store the physics process type.

      //Decay-in-flight
      if (processSubType >= static_cast<int>(DECAY) && processSubType <= static_cast<int>(DECAY_External))
        currParticle.setIgnore(false);  //Store the generator info in the MCParticles block.

    } else {
      //Unknown origin. This could be a bug originated from Geant4.
      currParticle.setSecondaryPhysicsProcess(-1);
    }

    //Either we store all the trajectories
    if (m_storeTrajectories > 2 ||
        //Or only the primary ones
        (m_storeTrajectories == 1 && dynamicParticle->GetPrimaryParticle() != NULL) ||
        //Or all except optical photons
        (m_storeTrajectories == 2 && track->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition())) {
      TrackInfo& info = dynamic_cast<TrackInfo&>(*track->GetUserInformation());
      m_relMCTrajectories.add(track->GetTrackID(), m_storeMCTrajectories.getEntries());
      info.setTrajectory(m_storeMCTrajectories.appendNew());
    }

  } catch (CouldNotFindUserInfo& exc) {
    B2FATAL(exc.what());
  }
}


void TrackingAction::PostUserTrackingAction(const G4Track* track)
{
  G4StepPoint* postStep = track->GetStep()->GetPostStepPoint();

  // Get particle of the current track
  try {
    MCParticleGraph::GraphParticle& currParticle = TrackInfo::getInfo(*track);

    // Add particle and decay Information to all the secondaries
    for (G4Track* daughterTrack : *fpTrackingManager->GimmeSecondaries()) {

      // Add the particle to the particle graph and as UserInfo to the track
      // if it is a secondary particle created by Geant4.
      if (daughterTrack->GetDynamicParticle()->GetPrimaryParticle() == NULL &&
          daughterTrack->GetUserInformation() == NULL) {
        MCParticleGraph::GraphParticle& daughterParticle = m_mcParticleGraph.addParticle();
        const_cast<G4Track*>(daughterTrack)->SetUserInformation(new TrackInfo(daughterParticle));

        currParticle.decaysInto(daughterParticle); //Add the decay history

        // Optical photons and secondaries:  steering of output to MCParticles
        if (daughterTrack->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {

          // Optical photons
          if (m_ignoreOpticalPhotons) daughterParticle.setIgnore();
          // to apply quantum efficiency only once, if optical photon is a daugher of optical photon
          if (track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
            TrackInfo* currInfo = dynamic_cast<TrackInfo*>(track->GetUserInformation());
            TrackInfo* daughterInfo = dynamic_cast<TrackInfo*>(daughterTrack->GetUserInformation());
            daughterInfo->setStatus(currInfo->getStatus());
            daughterInfo->setFraction(currInfo->getFraction());
          }

        } else if (daughterTrack->GetCreatorProcess()->GetProcessSubType() == fBremsstrahlung) {

          // Bremsstrahlung photons
          // Do not store the generator info in the final MCParticles block
          // if the ignore flag is set or its energy is too low in [MeV].
          if (m_ignoreBremsstrahlungPhotons || daughterTrack->GetKineticEnergy() < m_bremsstrahlungPhotonsEnergyCut)
            daughterParticle.setIgnore();

        } else if (daughterTrack->GetCreatorProcess()->GetProcessSubType() == fGammaConversion) {

          // e+ or e- created by gamma conversion to pairs
          // Do not store the generator info in the final MCParticles block
          // if the ignore flag is set or kinetic energy is too low in [MeV].
          if (m_ignorePairConversions || daughterTrack->GetKineticEnergy() < m_pairConversionsEnergyCut)
            daughterParticle.setIgnore();

        } else {

          // Do not store the generator info in the final MCParticles block
          // if the ignore flag is set or its kinetic energy is too low in [MeV].
          if (m_ignoreSecondaries || daughterTrack->GetKineticEnergy() < m_secondariesEnergyCut)
            daughterParticle.setIgnore();

          //B2INFO("Secondary Physics Process: " << daughterTrack->GetCreatorProcess()->GetProcessSubType());
        }

      }
    }
    //If the track is just suspended we can return here: the rest should be filled once the track is done
    if (track->GetTrackStatus() == fSuspend) return;

    //Check if particle left the detector/simulation volume.
    if (postStep->GetStepStatus() == fWorldBoundary) {
      currParticle.addStatus(MCParticle::c_LeftDetector);
    }

    //Check if particle was stopped in the detector/simulation volume
    if (track->GetKineticEnergy() <= 0.0) {
      currParticle.addStatus(MCParticle::c_StoppedInDetector);
    }

    //Set the values for the particle
    G4ThreeVector decVtx = postStep->GetPosition() / CLHEP::mm * Unit::mm;
    currParticle.setDecayVertex(decVtx.x(), decVtx.y(), decVtx.z());
    currParticle.setDecayTime(postStep->GetGlobalTime()); // Time does not need a conversion factor
    currParticle.setValidVertex(true);

    //Check if we can remove some points from the trajectory
    if (m_storeTrajectories) {
      MCParticleTrajectory* tr = dynamic_cast<TrackInfo*>(track->GetUserInformation())->getTrajectory();
      if (tr) {
        tr->simplify(m_distanceTolerance);
      }
    }
  } catch (CouldNotFindUserInfo& exc) {
    B2FATAL(exc.what());
  }
}
