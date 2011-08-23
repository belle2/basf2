/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <bklm/simulation/SensitiveDetector.h>
#include <bklm/simulation/SimulationPar.h>
#include <bklm/geometry/GeometryPar.h>
#include <bklm/geometry/Sector.h>
#include <bklm/geometry/Module.h>
#include <bklm/dataobjects/BKLMSimHit.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <boost/lexical_cast.hpp>

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include "TRandom3.h"

#include "G4Step.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4Neutron.hh"

using namespace std;

namespace Belle2 {

  namespace bklm {

    SensitiveDetector::SensitiveDetector() : SensitiveDetectorBase("BKLM", KLM)
    {
      m_FirstCall = true;
      StoreArray<MCParticle> particles;
      StoreArray<BKLMSimHit> simHits;
      RelationArray particleToSimHits(particles, simHits);
      registerMCParticleRelation(particleToSimHits);
    }

    void SensitiveDetector::Initialize(G4HCofThisEvent*)
    {
      m_HitNumber = 0;
      // One-time initializations (constructor is called too early)
      if (m_FirstCall) {
        m_FirstCall = false;
        m_NeutronPID = G4Neutron::Definition()->GetPDGEncoding(); // 2112
        SimulationPar* simPar = SimulationPar::instance();
        if (!simPar->isValid()) {
          B2FATAL("BKLM SensitiveDetector: simulation-control parameters are not available from module BKLMParamLoader")
        }
        m_HitTimeMax = simPar->getHitTimeMax();
        m_DoBackgroundStudy = simPar->getDoBackgroundStudy();
        m_Random = new TRandom3(simPar->getRandomSeed());
      }
    }

    //-----------------------------------------------------
    // Method invoked for every step in sensitive detector
    //-----------------------------------------------------
    G4bool SensitiveDetector::step(G4Step* step, G4TouchableHistory*)
    {

      // It is not necessary to detect motion from one volume to another (or track death
      // in the RPC gas volume).  Experimentation shows that most tracks pass through the
      // RPC gas volume in one step (although, on occasion, a delta ray will take a couple
      // of short steps entirely within gas).  Therefore, save every step in the gas
      // instead of trying to find entry and exit points and then saving only the midpoint.
      // Do same for scintillators.

      double       deltaE     = step->GetTotalEnergyDeposit();  // GEANT4: in MeV
      G4StepPoint* preStep    = step->GetPreStepPoint();
      G4StepPoint* postStep   = step->GetPostStepPoint();
      G4Track*     track      = step->GetTrack();
      int          primaryPID = track->GetDefinition()->GetPDGEncoding();

      // Record a step for a charged track that deposits some energy.
      // Background study: Record every neutron passage, whether it deposits energy or not.
      if (((deltaE > 0.0) && (postStep->GetCharge() != 0)) ||
          ((m_DoBackgroundStudy) && (primaryPID == m_NeutronPID))) {
        double KE = 0.5 * (preStep->GetKineticEnergy() + postStep->GetKineticEnergy());
        const G4ThreeVector g4HitPos = 0.5 * (preStep->GetPosition() + postStep->GetPosition()) / cm; // GEANT4: in mm
        const TVector3 hitPos(g4HitPos.x(), g4HitPos.y(), g4HitPos.z());
        double hitTime = 0.5 * (preStep->GetGlobalTime() + postStep->GetGlobalTime());  // GEANT4: in ns
        bool decayed = false;
        if (postStep->GetProcessDefinedStep() != 0) {
          decayed = (postStep->GetProcessDefinedStep()->GetProcessType() == fDecay);
        }
        const string volumeName = preStep->GetPhysicalVolume()->GetName();  // "BKLM.***_*_*_**_*"
        if (volumeName.find("BKLM.") != 0) {
          B2ERROR("BKLM SensitiveDetector: volume name (" << volumeName << ") is not \"BKLM.***_*_*_**_*\"");
          return false;
        }
        bool inRPC = (volumeName.find("BKLM.Gas") == 0);    // true in gas, false in scint
        int frontBack = boost::lexical_cast<int>(volumeName.substr(9, 1));
        int sector = boost::lexical_cast<int>(volumeName.substr(11, 1));
        int layer = boost::lexical_cast<int>(volumeName.substr(13, 2));
        int plane = boost::lexical_cast<int>(volumeName.substr(16, 1));
        StoreArray<BKLMSimHit> simHits;
        BKLMSimHit* simHit = new(simHits->AddrAt(m_HitNumber))
        BKLMSimHit(hitPos, hitTime, deltaE, KE, inRPC, decayed, frontBack, sector, layer, plane);
        if (inRPC) {
          convertHitToRPCStrips(simHit);
        } else {
          int strip = boost::lexical_cast<int>(volumeName.substr(12, 2));
          if (plane == 0) {
            simHit->appendPhiStrip(strip);
          } else {
            simHit->appendZStrip(strip);
          }
        }
        StoreArray<MCParticle> particles;
        RelationArray particleToSimHits(particles, simHits);
        particleToSimHits.add(track->GetTrackID(), m_HitNumber);
        m_HitNumber++;
        return true;
      }
      return false;
    }

    void SensitiveDetector::convertHitToRPCStrips(BKLMSimHit* simHit)
    {
      const GeometryPar* geoPar = GeometryPar::instance();
      const SimulationPar* simPar = SimulationPar::instance();
      int frontBack = simHit->getFrontBack();
      int sector = simHit->getSector();
      int layer = simHit->getLayer();
      const Sector* pS = geoPar->findSector(frontBack, sector);
      const Module* pM = pS->findModule(layer);
      const TVector3 globalPos = simHit->getHitPosition();
      const CLHEP::Hep3Vector localPos = pS->globalToLocal(globalPos.X(),
                                                           globalPos.Y(),
                                                           globalPos.Z());
      int phiStrip = 0;
      int zStrip = 0;
      if (pM->isInActiveArea(localPos, phiStrip, zStrip)) {
        simHit->appendPhiStrip(phiStrip);
        simHit->appendZStrip(zStrip);
        double phiStripDiv = 0.0; // between -0.5 and +0.5 within central phiStrip
        double zStripDiv = 0.0;   // between -0.5 and +0.5 within central zStrip
        pM->getStripDivisions(localPos, phiStripDiv, zStripDiv);
        int n = 0;
        double rand = m_Random->Uniform();
        for (n = 1; n < simPar->getMaxMultiplicity(); ++n) {
          if (simPar->getPhiMultiplicityCDF(phiStripDiv, n) > rand) break;
        }
        int nextStrip = (phiStripDiv > 0.0 ? 1 : -1);
        while (--n > 0) {
          phiStrip += nextStrip;
          if ((phiStrip >= pM->getPhiStripMin()) && (phiStrip <= pM->getPhiStripMax())) {
            simHit->appendPhiStrip(phiStrip);
          }
          nextStrip = (nextStrip > 0 ? -(1 + nextStrip) : 1 - nextStrip);
        }
        rand = m_Random->Uniform();
        for (n = 1; n < simPar->getMaxMultiplicity(); ++n) {
          if (simPar->getZMultiplicityCDF(zStripDiv, n) > rand) break;
        }
        nextStrip = (zStripDiv > 0.0 ? 1 : -1);
        while (--n > 0) {
          zStrip += nextStrip;
          if ((zStrip >= pM->getZStripMin()) && (zStrip <= pM->getZStripMax())) {
            simHit->appendZStrip(zStrip);
          }
          nextStrip = (nextStrip > 0 ? -(1 + nextStrip) : 1 - nextStrip);
        }
      }
      return;
    }
  } // end of namespace bklm
} // end of namespace Belle2
