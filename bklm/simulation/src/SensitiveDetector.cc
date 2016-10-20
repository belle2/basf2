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
#include <bklm/geometry/Module.h>
#include <bklm/dataobjects/BKLMSimHit.h>
#include <bklm/dataobjects/BKLMSimHitPosition.h>
#include <bklm/dataobjects/BKLMStatus.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <simulation/background/BkgSensitiveDetector.h>
#include <mdst/dataobjects/MCParticle.h>

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Units/PhysicalConstants.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include "TRandom3.h"

#include "G4Step.hh"
#include "G4VProcess.hh"

#define DEPTH_FORWARD 2
#define DEPTH_SECTOR 3
#define DEPTH_LAYER 5
#define DEPTH_PLANE 9
#define DEPTH_SCINT 10

using namespace std;

namespace Belle2 {

  namespace bklm {

    SensitiveDetector::SensitiveDetector(const G4String& name) : SensitiveDetectorBase(name, Const::KLM)
    {
      m_FirstCall = true;
      m_HitTimeMax = 0.0;
      m_BkgSensitiveDetector = NULL;
      m_GeoPar = NULL;
      m_SimPar = NULL;
      StoreArray<MCParticle> particles;
      StoreArray<BKLMSimHit> simHits;
      StoreArray<BKLMSimHitPosition> simHitPositions;
      simHits.registerInDataStore();
      simHitPositions.registerInDataStore();
      particles.registerRelationTo(simHits);
      simHitPositions.registerRelationTo(simHits);
      RelationArray particleToSimHits(particles, simHits);
      registerMCParticleRelation(particleToSimHits);
    }

    //-----------------------------------------------------
    // Method invoked for every step in sensitive detector
    //-----------------------------------------------------
    G4bool SensitiveDetector::step(G4Step* step, G4TouchableHistory* history)
    {

      // Once-only initializations (constructor is called too early for these)
      if (m_FirstCall) {
        m_FirstCall = false;
        m_GeoPar = GeometryPar::instance();
        if (m_GeoPar->doBeamBackgroundStudy()) {
          m_BkgSensitiveDetector = m_GeoPar->getBkgSensitiveDetector();
        }
        m_SimPar = SimulationPar::instance();
        if (!(m_SimPar->isValid())) {
          B2FATAL("Simulation-control parameters are not available from module BKLMParamLoader");
        }
        m_HitTimeMax = m_SimPar->getHitTimeMax();
        if (!gRandom) B2FATAL("gRandom is not initialized; please set up gRandom first");
      }

      // Record a BeamBackHit for any particle
      if (m_BkgSensitiveDetector != NULL) {
        m_BkgSensitiveDetector->step(step, history);
      }

      StoreArray<BKLMSimHit> simHits;
      StoreArray<BKLMSimHitPosition> simHitPositions;
      StoreArray<MCParticle> particles;
      RelationArray particleToSimHits(particles, simHits);

      // It is not necessary to detect motion from one volume to another (or track death
      // in the RPC gas volume).  Experimentation shows that most tracks pass through the
      // RPC gas volume in one step (although, on occasion, a delta ray will take a couple
      // of short steps entirely within gas).  Therefore, save every step in the gas
      // instead of trying to find entry and exit points and then saving only the midpoint.
      // Do same for scintillators.

      double       eDep     = step->GetTotalEnergyDeposit() / CLHEP::MeV;  // GEANT4: in MeV
      G4StepPoint* preStep  = step->GetPreStepPoint();
      G4StepPoint* postStep = step->GetPostStepPoint();
      G4Track*     track    = step->GetTrack();

      // Record a BKLMSimHit for a charged track that deposits some energy.
      if ((eDep > 0.0) && (postStep->GetCharge() != 0.0)) {
        const G4VTouchable* hist = preStep->GetTouchable();
        int depth = hist->GetHistoryDepth();
        if (depth < DEPTH_PLANE) {
          B2WARNING("Touchable History depth = " << depth << " should be at least " << DEPTH_PLANE);
          return false;
        }
        int plane = hist->GetCopyNumber(depth - DEPTH_PLANE);
        int layer = hist->GetCopyNumber(depth - DEPTH_LAYER);
        int sector = hist->GetCopyNumber(depth - DEPTH_SECTOR);
        bool isForward = (hist->GetCopyNumber(depth - DEPTH_FORWARD) == BKLM_FORWARD);
        int moduleID = (isForward ? BKLM_END_MASK : 0)
                       | ((sector - 1) << BKLM_SECTOR_BIT)
                       | ((layer - 1) << BKLM_LAYER_BIT)
                       | BKLM_MC_MASK;
        double time = 0.5 * (preStep->GetGlobalTime() + postStep->GetGlobalTime());  // GEANT4: in ns
        const CLHEP::Hep3Vector globalPosition = 0.5 * (preStep->GetPosition() + postStep->GetPosition()) / CLHEP::cm; // in cm
        const Module* m = m_GeoPar->findModule(isForward, sector, layer);
        const CLHEP::Hep3Vector localPosition = m->globalToLocal(globalPosition);
        const CLHEP::Hep3Vector propagationTimes = m->getPropagationTimes(localPosition);
        if (postStep->GetProcessDefinedStep() != 0) {
          if (postStep->GetProcessDefinedStep()->GetProcessType() == fDecay) { moduleID |= BKLM_DECAYED_MASK; }
        }
        int trackID = track->GetTrackID();
        if (m->hasRPCs()) {
          moduleID |= BKLM_INRPC_MASK;
          int phiStripLower = -1;
          int phiStripUpper = -1;
          int zStripLower = -1;
          int zStripUpper = -1;
          convertHitToRPCStrips(localPosition, m, phiStripLower, phiStripUpper, zStripLower, zStripUpper);
          if (zStripLower > 0) {
            int moduleIDZ = moduleID | ((zStripLower - 1) << BKLM_STRIP_BIT) | ((zStripUpper - 1) << BKLM_MAXSTRIP_BIT);
            BKLMSimHit* simHit = simHits.appendNew(moduleIDZ, propagationTimes.z(), time, eDep);
            particleToSimHits.add(trackID, simHits.getEntries() - 1);
            BKLMSimHitPosition* simHitPosition = simHitPositions.appendNew(globalPosition.x(), globalPosition.y(), globalPosition.z());
            simHitPosition->addRelationTo(simHit);
          }
          if (phiStripLower > 0) {
            moduleID |= ((phiStripLower - 1) << BKLM_STRIP_BIT) | ((phiStripUpper - 1) << BKLM_MAXSTRIP_BIT) | BKLM_PLANE_MASK;
            BKLMSimHit* simHit = simHits.appendNew(moduleID, propagationTimes.y(), time, eDep);
            particleToSimHits.add(trackID, simHits.getEntries() - 1);
            BKLMSimHitPosition* simHitPosition = simHitPositions.appendNew(globalPosition.x(), globalPosition.y(), globalPosition.z());
            simHitPosition->addRelationTo(simHit);
          }
        } else {
          int scint = hist->GetCopyNumber(depth - DEPTH_SCINT);
          moduleID |= ((scint - 1) << BKLM_STRIP_BIT) | ((scint - 1) << BKLM_MAXSTRIP_BIT);
          double propTime = propagationTimes.z();
          if (plane == BKLM_INNER) {
            moduleID |= BKLM_PLANE_MASK;
            propTime = propagationTimes.y();
          }
          BKLMSimHit* simHit = simHits.appendNew(moduleID, propTime, time, eDep);
          particleToSimHits.add(trackID, simHits.getEntries() - 1);
          BKLMSimHitPosition* simHitPosition = simHitPositions.appendNew(globalPosition.x(), globalPosition.y(), globalPosition.z());
          simHitPosition->addRelationTo(simHit);
        }
        return true;
      }
      return false;
    }

    void SensitiveDetector::convertHitToRPCStrips(const CLHEP::Hep3Vector& localPosition, const Module* m,
                                                  int& phiStripLower, int& phiStripUpper, int& zStripLower, int& zStripUpper)
    {
      double phiStripD = m->getPhiStrip(localPosition);
      int phiStrip = int(phiStripD);
      int pMin = m->getPhiStripMin();
      if (phiStrip < pMin) return;
      int pMax = m->getPhiStripMax();
      if (phiStrip > pMax) return;

      double zStripD = m->getZStrip(localPosition);
      int zStrip = int(zStripD);
      int zMin = m->getZStripMin();
      if (zStrip < zMin) return;
      int zMax = m->getZStripMax();
      if (zStrip > zMax) return;

      phiStripLower = phiStrip;
      phiStripUpper = phiStrip;
      zStripLower = zStrip;
      zStripUpper = zStrip;
      double phiStripDiv = fmod(phiStripD, 1.0) - 0.5; // between -0.5 and +0.5 within central phiStrip
      double zStripDiv = fmod(zStripD, 1.0) - 0.5;   // between -0.5 and +0.5 within central zStrip
      int n = 0;
      double rand = gRandom->Uniform();
      for (n = 1; n < m_SimPar->getMaxMultiplicity(); ++n) {
        if (m_SimPar->getPhiMultiplicityCDF(phiStripDiv, n) > rand) break;
      }
      int nextStrip = (phiStripDiv > 0.0 ? 1 : -1);
      while (--n > 0) {
        phiStrip += nextStrip;
        if ((phiStrip >= pMin) && (phiStrip <= pMax)) {
          phiStripLower = min(phiStrip, phiStripLower);
          phiStripUpper = max(phiStrip, phiStripUpper);
        }
        nextStrip = (nextStrip > 0 ? -(1 + nextStrip) : 1 - nextStrip);
      }
      rand = gRandom->Uniform();
      for (n = 1; n < m_SimPar->getMaxMultiplicity(); ++n) {
        if (m_SimPar->getZMultiplicityCDF(zStripDiv, n) > rand) break;
      }
      nextStrip = (zStripDiv > 0.0 ? 1 : -1);
      while (--n > 0) {
        zStrip += nextStrip;
        if ((zStrip >= zMin) && (zStrip <= zMax)) {
          zStripLower = min(zStrip, zStripLower);
          zStripUpper = max(zStrip, zStripUpper);
        }
        nextStrip = (nextStrip > 0 ? -(1 + nextStrip) : 1 - nextStrip);
      }
      return;
    }
  } // end of namespace bklm
} // end of namespace Belle2
