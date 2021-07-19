/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/simulation/SensitivePMT.h>

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

using namespace std;

namespace Belle2 {
  namespace TOP {

    SensitivePMT::SensitivePMT():
      Simulation::SensitiveDetectorBase("TOP", Const::TOP)
    {

      m_simHits.registerInDataStore();
      m_mcParticles.registerRelationTo(m_simHits);

      m_simPhotons.registerInDataStore(DataStore::c_DontWriteOut);
      m_simHits.registerRelationTo(m_simPhotons, DataStore::c_Event,
                                   DataStore::c_DontWriteOut);

      registerMCParticleRelation(m_relParticleHit);

    }


    G4bool SensitivePMT::step(G4Step* aStep, G4TouchableHistory*)
    {
      // photon track
      G4Track& photon  = *aStep->GetTrack();

      // check if the track is an optical photon
      if (photon.GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()) return false;

      // photon energy in [eV]
      double energy = photon.GetKineticEnergy() * Unit::MeV / Unit::eV;

      // pmt and module ID
      int pmtID = photon.GetTouchableHandle()->GetReplicaNumber(m_pmtReplicaDepth);
      int moduleID = photon.GetTouchableHandle()->GetReplicaNumber(m_moduleReplicaDepth);

      // hit position in local frame, converted to Basf units
      G4ThreeVector localPosition = photon.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(photon.GetPosition());
      double xLocal = localPosition.x() * Unit::mm;
      double yLocal = localPosition.y() * Unit::mm;

      // apply quantum efficiency if not yet done
      bool applyQE = true;
      double fraction = 1;
      auto* info = dynamic_cast<Simulation::TrackInfo*>(photon.GetUserInformation());
      if (info) {
        applyQE = info->getStatus() < 3;
        fraction = info->getFraction();
      }
      if (applyQE) {
        double qeffi = m_topgp->getPMTEfficiency(energy, moduleID, pmtID, xLocal, yLocal);
        if (qeffi == 0 or gRandom->Uniform() * fraction > qeffi) {
          photon.SetTrackStatus(fStopAndKill);
          return false;
        }
        info->setStatus(3);
        info->setFraction(qeffi);
      }

      // photon at detection
      const G4ThreeVector& g_detPoint = photon.GetPosition();
      const G4ThreeVector& g_detMomDir = photon.GetMomentumDirection();
      TVector3 detPoint(g_detPoint.x(), g_detPoint.y(), g_detPoint.z());
      TVector3 detMomDir(g_detMomDir.x(), g_detMomDir.y(), g_detMomDir.z());
      double detTime = photon.GetGlobalTime();
      double length = photon.GetTrackLength();

      // photon at emission
      const G4ThreeVector& g_emiPoint = photon.GetVertexPosition();
      const G4ThreeVector& g_emiMomDir = photon.GetVertexMomentumDirection();
      TVector3 emiPoint(g_emiPoint.x(), g_emiPoint.y(), g_emiPoint.z());
      TVector3 emiMomDir(g_emiMomDir.x(), g_emiMomDir.y(), g_emiMomDir.z());
      double emiTime = photon.GetGlobalTime() - photon.GetLocalTime();

      // convert to basf2 units
      emiPoint = emiPoint * Unit::mm;
      detPoint = detPoint * Unit::mm;
      length = length * Unit::mm;

      // write to store arrays; add relations

      auto* simHit = m_simHits.appendNew(moduleID, pmtID, xLocal, yLocal,
                                         detTime, energy);

      int parentID = photon.GetParentID();
      if (parentID == 0) parentID = photon.GetTrackID();
      m_relParticleHit.add(parentID, simHit->getArrayIndex());

      const auto* geo = m_topgp->getGeometry();
      if (geo->isModuleIDValid(moduleID)) {
        // transform to local frame
        const auto& module = geo->getModule(moduleID);
        emiPoint = module.pointToLocal(emiPoint);
        detPoint = module.pointToLocal(detPoint);
        emiMomDir = module.momentumToLocal(emiMomDir);
        detMomDir = module.momentumToLocal(detMomDir);
      } else {
        B2ERROR("TOP::SensitivePMT: undefined module ID."
                << LogVar("moduleID", moduleID));
      }
      auto* simPhoton = m_simPhotons.appendNew(moduleID,
                                               emiPoint, emiMomDir, emiTime,
                                               detPoint, detMomDir, detTime,
                                               length, energy);

      simHit->addRelationTo(simPhoton);

      // kill photon after detection
      photon.SetTrackStatus(fStopAndKill);

      return true;
    }


  } // end of namespace top
} // end of namespace Belle2
