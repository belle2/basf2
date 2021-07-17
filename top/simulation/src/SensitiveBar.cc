/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/simulation/SensitiveBar.h>

#include <simulation/kernel/UserInfo.h>
#include <G4Step.hh>
#include <G4Track.hh>
#include <G4UnitsTable.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTypes.hh>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <TVector3.h>
#include <TRandom.h>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace TOP {

    SensitiveBar::SensitiveBar():
      Simulation::SensitiveDetectorBase("TOP", Const::TOP)
    {

      // registration of store arrays and relations

      m_barHits.registerInDataStore();
      m_mcParticles.registerRelationTo(m_barHits);

      // additional registration of MCParticle relation (required for correct relations)

      registerMCParticleRelation(m_relParticleHit, RelationArray::c_deleteElement);

      const auto* geo = m_topgp->getGeometry();
      m_trackIDs.resize(geo->getNumModules(), 0);

    }


    bool SensitiveBar::step(G4Step* aStep, G4TouchableHistory*)
    {

      // get track and particle definition

      G4Track* aTrack = aStep->GetTrack();
      G4ParticleDefinition* particle = aTrack->GetDefinition();

      // if optical photon, apply QE and return false

      if (particle == G4OpticalPhoton::OpticalPhotonDefinition()) {
        auto* info = dynamic_cast<Simulation::TrackInfo*>(aTrack->GetUserInformation());
        if (!info) return false;
        if (info->getStatus() < 2) {
          double energy = aTrack->GetKineticEnergy() * Unit::MeV / Unit::eV;
          double qeffi = m_topgp->getPMTEfficiencyEnvelope(energy);
          double fraction = info->getFraction();
          if (qeffi == 0 or gRandom->Uniform() * fraction > qeffi) {
            aTrack->SetTrackStatus(fStopAndKill);
            return false;
          }
          info->setStatus(2);
          info->setFraction(qeffi);
        }
        return false;
      }

      // continue for other particles, if they enter the volume for the first time

      G4StepPoint* PrePosition =  aStep->GetPreStepPoint();
      if (PrePosition->GetStepStatus() != fGeomBoundary) return false; // not on boundary

      if (m_barHits.getEntries() == 0) {
        for (auto& trackID : m_trackIDs) trackID = -1; // reset on new event
      }

      int moduleID = PrePosition->GetTouchableHandle()->GetReplicaNumber(m_replicaDepth);
      const auto* geo = m_topgp->getGeometry();
      if (!geo->isModuleIDValid(moduleID)) {
        B2ERROR("TOP::SensitiveBar: undefined module ID."
                << LogVar("moduleID", moduleID));
        return false;
      }

      int trackID = aTrack->GetTrackID();
      if (trackID == m_trackIDs[moduleID - 1]) return false; // not first time on boundary
      m_trackIDs[moduleID - 1] = trackID;

      // particle other than optical photon entered the volume for the first time
      // convert to basf2 units and write-out the hit

      G4ThreeVector worldPosition = PrePosition->GetPosition();
      double tracklength = aTrack->GetTrackLength() - aStep->GetStepLength();
      double globalTime = PrePosition->GetGlobalTime();
      G4ThreeVector momentum = PrePosition->GetMomentum();

      TVector3 TPosition(worldPosition.x(), worldPosition.y(), worldPosition.z());
      TVector3 TMomentum(momentum.x(), momentum.y(), momentum.z());
      TVector3 TOrigin(aTrack->GetVertexPosition().x(),
                       aTrack->GetVertexPosition().y(),
                       aTrack->GetVertexPosition().z());

      // convert to basf2 units
      TPosition = TPosition * Unit::mm;
      TMomentum = TMomentum  * Unit::MeV;
      TOrigin = TOrigin * Unit::mm;
      tracklength = tracklength * Unit::mm;

      const auto& module = geo->getModule(moduleID);
      TVector3 locPosition = module.pointToLocal(TPosition);
      TVector3 locMomentum = module.momentumToLocal(TMomentum);
      double theta = locMomentum.Theta();
      double phi = locMomentum.Phi();

      int PDG = (int)(particle->GetPDGEncoding());

      // write hit to datastore
      auto* hit = m_barHits.appendNew(moduleID, PDG, TOrigin, TPosition, TMomentum,
                                      globalTime, tracklength, locPosition,
                                      theta, phi);

      // set the relation
      m_relParticleHit.add(trackID, hit->getArrayIndex());

      return true;
    }


  } // end of namespace top
} // end of namespace Belle2
