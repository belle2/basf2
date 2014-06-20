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

#include <mdst/dataobjects/MCParticle.h>

#include <boost/lexical_cast.hpp>

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Units/PhysicalConstants.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include "TRandom3.h"

#include "G4Step.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4Neutron.hh"

#define DEPTH_RPC 9
#define DEPTH_SCINT 10

using namespace std;

namespace Belle2 {

  namespace bklm {

    SensitiveDetector::SensitiveDetector() : SensitiveDetectorBase("BKLM", Const::KLM)
    {
      m_FirstCall = true;
      m_NeutronPDG = 0;     // dummy initializer
      m_DoBackgroundStudy = false;  // dummy initializer
      m_HitTimeMax = 0.0;   // dummy initializer
      m_GeoPar = NULL;  // dummy initializer
      m_SimPar = NULL;  // dummy initializer
      StoreArray<MCParticle> particles;
      StoreArray<BKLMSimHit> simHits;
      StoreArray<BKLMSimHitPosition> simHitPositions;
      RelationArray particleToSimHits(particles, simHits);
      registerMCParticleRelation(particleToSimHits);
      StoreArray<BKLMSimHit>::registerPersistent();
      StoreArray<BKLMSimHitPosition>::registerPersistent();
      RelationArray::registerPersistent<MCParticle, BKLMSimHit>();
      RelationArray::registerPersistent<BKLMSimHitPosition, BKLMSimHit>();
    }

    //-----------------------------------------------------
    // Method invoked for every step in sensitive detector
    //-----------------------------------------------------
    G4bool SensitiveDetector::step(G4Step* step, G4TouchableHistory*)
    {

      // Once-only initializations (constructor is called too early for these)
      if (m_FirstCall) {
        m_FirstCall = false;
        m_GeoPar = GeometryPar::instance();
        m_SimPar = SimulationPar::instance();
        if (!(m_SimPar->isValid())) {
          B2FATAL("Simulation-control parameters are not available from module BKLMParamLoader")
        }
        m_HitTimeMax = m_SimPar->getHitTimeMax();
        m_DoBackgroundStudy = m_SimPar->getDoBackgroundStudy();
        m_NeutronPDG = G4Neutron::Definition()->GetPDGEncoding(); // =2112
        if (!gRandom) B2FATAL("gRandom is not initialized; please set up gRandom first");
      }
      StoreArray<BKLMSimHit> simHits;
      if (!simHits.isValid()) simHits.create();
      StoreArray<BKLMSimHitPosition> simHitPositions;
      if (!simHitPositions.isValid()) simHitPositions.create();
      StoreArray<MCParticle> particles;
      RelationArray particleToSimHits(particles, simHits);
      RelationArray positionToSimHits(simHitPositions, simHits);

      // It is not necessary to detect motion from one volume to another (or track death
      // in the RPC gas volume).  Experimentation shows that most tracks pass through the
      // RPC gas volume in one step (although, on occasion, a delta ray will take a couple
      // of short steps entirely within gas).  Therefore, save every step in the gas
      // instead of trying to find entry and exit points and then saving only the midpoint.
      // Do same for scintillators.

      double       eDep     = step->GetTotalEnergyDeposit() / MeV;  // GEANT4: in MeV
      G4StepPoint* preStep  = step->GetPreStepPoint();
      G4StepPoint* postStep = step->GetPostStepPoint();
      G4Track*     track    = step->GetTrack();
      int          pdg      = track->GetDefinition()->GetPDGEncoding();

      // Record a step for a charged track that deposits some energy.
      // Background study: Record every neutron passage, whether it deposits energy or not.
      if (((eDep > 0.0) && (postStep->GetCharge() != 0.0)) ||
          ((m_DoBackgroundStudy) && (pdg == m_NeutronPDG))) {
        const G4VTouchable* hist = preStep->GetTouchable();
        int baseDepth = hist->GetHistoryDepth() - DEPTH_RPC;
        if ((baseDepth < 0) || (baseDepth > DEPTH_SCINT - DEPTH_RPC)) {
          B2WARNING("Touchable History baseDepth = " << baseDepth + DEPTH_RPC << " (should be 9=RPC or 10=scint)")
          return false;
        }
        int plane = hist->GetCopyNumber(baseDepth);
        int layer = hist->GetCopyNumber(baseDepth + 4);
        int sector = hist->GetCopyNumber(baseDepth + 6);
        bool isForward = (hist->GetCopyNumber(baseDepth + 7) == BKLM_FORWARD);
        int moduleID = (isForward ? BKLM_END_MASK : 0)
                       | ((sector - 1) << BKLM_SECTOR_BIT)
                       | ((layer - 1) << BKLM_LAYER_BIT)
                       | BKLM_MC_MASK;
        const CLHEP::Hep3Vector mom = 0.5 * (preStep->GetMomentum() + postStep->GetMomentum());  // GEANT4: in MeV/c
        const TVector3 momentum(mom.x(), mom.y(), mom.z());
        double time = 0.5 * (preStep->GetGlobalTime() + postStep->GetGlobalTime());  // GEANT4: in ns
        const CLHEP::Hep3Vector gHitPos = 0.5 * (preStep->GetPosition() + postStep->GetPosition()) / cm; // in cm
        const Module* m = m_GeoPar->findModule(isForward, sector, layer);
        const CLHEP::Hep3Vector lHitPos = m->globalToLocal(gHitPos);
        if (postStep->GetProcessDefinedStep() != 0) {
          if (postStep->GetProcessDefinedStep()->GetProcessType() == fDecay) { moduleID |= BKLM_DECAYED_MASK; }
        }
        int trackID = track->GetTrackID();
        if (baseDepth == 0) {
          moduleID |= BKLM_INRPC_MASK;
          int phiStripLower = -1;
          int phiStripUpper = -1;
          int zStripLower = -1;
          int zStripUpper = -1;
          convertHitToRPCStrips(lHitPos, m, phiStripLower, phiStripUpper, zStripLower, zStripUpper);
          if (zStripLower > 0) {
            int moduleIDZ = moduleID | ((zStripLower - 1) << BKLM_STRIP_BIT) | ((zStripUpper - 1) << BKLM_MAXSTRIP_BIT);
            new(simHits.nextFreeAddress()) BKLMSimHit(moduleIDZ, lHitPos.x(), time, eDep);
            particleToSimHits.add(trackID, simHits.getEntries() - 1);
            new(simHitPositions.nextFreeAddress()) BKLMSimHitPosition(gHitPos.x(), gHitPos.y(), gHitPos.z());
            positionToSimHits.add(simHitPositions.getEntries() - 1, simHits.getEntries() - 1);
          }
          if (phiStripLower > 0) {
            moduleID |= ((phiStripLower - 1) << BKLM_STRIP_BIT) | ((phiStripUpper - 1) << BKLM_MAXSTRIP_BIT) | BKLM_PLANE_MASK;
            new(simHits.nextFreeAddress()) BKLMSimHit(moduleID, lHitPos.x(), time, eDep);
            particleToSimHits.add(trackID, simHits.getEntries() - 1);
            new(simHitPositions.nextFreeAddress()) BKLMSimHitPosition(gHitPos.x(), gHitPos.y(), gHitPos.z());
            positionToSimHits.add(simHitPositions.getEntries() - 1, simHits.getEntries() - 1);
          }
        } else {
          int scint = hist->GetCopyNumber(0);
          moduleID |= ((scint - 1) << BKLM_STRIP_BIT) | ((scint - 1) << BKLM_MAXSTRIP_BIT);
          if (plane == BKLM_INNER) moduleID |= BKLM_PLANE_MASK;
          new(simHits.nextFreeAddress()) BKLMSimHit(moduleID, lHitPos.x(), time, eDep);
          particleToSimHits.add(trackID, simHits.getEntries() - 1);
          new(simHitPositions.nextFreeAddress()) BKLMSimHitPosition(gHitPos.x(), gHitPos.y(), gHitPos.z());
          positionToSimHits.add(simHitPositions.getEntries() - 1, simHits.getEntries() - 1);
        }
        return true;
      }
      return false;
    }

    void SensitiveDetector::convertHitToRPCStrips(const CLHEP::Hep3Vector& lHitPos, const Module* m,
                                                  int& phiStripLower, int& phiStripUpper, int& zStripLower, int& zStripUpper)
    {
      double phiStripD = m->getPhiStrip(lHitPos);
      int phiStrip = int(phiStripD);
      int pMin = m->getPhiStripMin();
      if (phiStrip < pMin) return;
      int pMax = m->getPhiStripMax();
      if (phiStrip > pMax) return;

      double zStripD = m->getZStrip(lHitPos);
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
