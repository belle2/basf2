/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/simulation/SensitivePMT.h>
#include <top/dataobjects/TOPSimHit.h>
#include <top/dataobjects/TOPSimPhoton.h>

#include <simulation/kernel/UserInfo.h>
#include <G4Step.hh>
#include <G4Track.hh>
#include <G4UnitsTable.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTypes.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <TVector3.h>
#include <TRandom3.h>

using namespace std;

namespace Belle2 {
  namespace TOP {

    SensitivePMT::SensitivePMT():
      Simulation::SensitiveDetectorBase("TOP", Const::TOP),
      m_topgp(TOPGeometryPar::Instance())
    {

      StoreArray<MCParticle> mcParticles;

      StoreArray<TOPSimHit> simHits;
      simHits.registerInDataStore();
      mcParticles.registerRelationTo(simHits);

      StoreArray<TOPSimPhoton> simPhotons;
      simPhotons.registerInDataStore(DataStore::c_DontWriteOut);
      simHits.registerRelationTo(simPhotons, DataStore::c_Event,
                                 DataStore::c_DontWriteOut);

      RelationArray  relation(mcParticles, simHits);
      registerMCParticleRelation(relation);

    }


    G4bool SensitivePMT::step(G4Step* aStep, G4TouchableHistory*)
    {
      // photon track
      G4Track& photon  = *aStep->GetTrack();

      // check if the track is an optical photon
      if (photon.GetDefinition()->GetParticleName() != "opticalphoton") return false;

      // apply quantum efficiency if not yet done
      bool applyQE = true;
      Simulation::TrackInfo* info =
        dynamic_cast<Simulation::TrackInfo*>(photon.GetUserInformation());
      if (info) applyQE = info->getStatus() < 2;
      if (applyQE) {
        double energy = photon.GetKineticEnergy() * Unit::MeV / Unit::eV;
        double qeffi = m_topgp->QE(energy) * m_topgp->getColEffi();
        double fraction = info->getFraction();
        if (gRandom->Uniform() * fraction > qeffi) {
          photon.SetTrackStatus(fStopAndKill);
          return false;
        }
      }

      // pmt and bar ID
      int pmtID = photon.GetTouchableHandle()->GetReplicaNumber(1);
      int barID = photon.GetTouchableHandle()->GetReplicaNumber(4);

      // photon at detection
      const G4ThreeVector& g_detPoint = photon.GetPosition();
      const G4ThreeVector& g_detMomDir = photon.GetMomentumDirection();
      TVector3 detPoint(g_detPoint.x(), g_detPoint.y(), g_detPoint.z());
      TVector3 detMomDir(g_detMomDir.x(), g_detMomDir.y(), g_detMomDir.z());
      double detTime = photon.GetGlobalTime();
      double energy = photon.GetKineticEnergy();
      double length = photon.GetTrackLength();

      // photon at emission
      const G4ThreeVector& g_emiPoint = photon.GetVertexPosition();
      const G4ThreeVector& g_emiMomDir = photon.GetVertexMomentumDirection();
      TVector3 emiPoint(g_emiPoint.x(), g_emiPoint.y(), g_emiPoint.z());
      TVector3 emiMomDir(g_emiMomDir.x(), g_emiMomDir.y(), g_emiMomDir.z());
      double emiTime = photon.GetGlobalTime() - photon.GetLocalTime();

      // convert to Basf units (photon energy in [eV]!)
      emiPoint = emiPoint * Unit::mm;
      detPoint = detPoint * Unit::mm;
      energy = energy * Unit::MeV / Unit::eV;
      length = length * Unit::mm;

      // hit position in local frame, converted to Basf units
      G4ThreeVector localPosition = photon.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(g_detPoint);
      double xLocal = localPosition.x() * Unit::mm;
      double yLocal = localPosition.y() * Unit::mm;

      // write to store arrays; add relations

      StoreArray<TOPSimHit> simHits;
      if (!simHits.isValid()) simHits.create();
      TOPSimHit* simHit = simHits.appendNew(barID, pmtID, xLocal, yLocal, detTime, energy);

      StoreArray<MCParticle> particles;
      RelationArray relParticleHit(particles, simHits);
      int parentID = photon.GetParentID();
      relParticleHit.add(parentID, simHit->getArrayIndex());

      StoreArray<TOPSimPhoton> simPhotons;
      if (!simPhotons.isValid()) simPhotons.create();
      TOPSimPhoton* simPhoton = simPhotons.appendNew(barID, emiPoint, emiMomDir, emiTime,
                                                     detPoint, detMomDir, detTime,
                                                     length, energy);

      simHit->addRelationTo(simPhoton);

      // kill photon after detection
      photon.SetTrackStatus(fStopAndKill);

      return true;
    }


  } // end of namespace top
} // end of namespace Belle2
