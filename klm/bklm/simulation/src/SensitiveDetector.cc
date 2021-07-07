/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/bklm/simulation/SensitiveDetector.h>

/* KLM headers. */
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/bklm/geometry/Module.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/bklm/BKLMStatus.h>

/* Belle 2 headers. */
#include <simulation/background/BkgSensitiveDetector.h>

/* Geant4 headers. */
#include <G4Step.hh>
#include <G4VProcess.hh>

/* CLHEP headers. */
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Vector/ThreeVector.h>

/* ROOT headers. */
#include <TRandom3.h>

using namespace std;
using namespace Belle2::bklm;

SensitiveDetector::SensitiveDetector(const G4String& name) :
  SensitiveDetectorBase(name, Const::KLM),
  m_FirstCall(true),
  m_BkgSensitiveDetector(nullptr),
  m_GeoPar(nullptr)
{
  if (!m_SimPar.isValid())
    B2FATAL("BKLM simulation parameters are not available.");
  m_HitTimeMax = m_SimPar->getHitTimeMax();
  m_MCParticles.isOptional();
  m_SimHits.registerInDataStore();
  m_SimHitPositions.registerInDataStore();
  m_MCParticlesToSimHits.registerInDataStore();
  registerMCParticleRelation(m_MCParticlesToSimHits);
  m_SimHitPositions.registerRelationTo(m_SimHits);
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
    if (m_GeoPar->doBeamBackgroundStudy())
      m_BkgSensitiveDetector = m_GeoPar->getBkgSensitiveDetector();
    if (!m_SimPar.isValid())
      B2FATAL("BKLM simulation parameters are not available.");
    m_HitTimeMax = m_SimPar->getHitTimeMax();
    if (!gRandom)
      B2FATAL("gRandom is not initialized; please set up gRandom first");
  }

  // Record a BeamBackHit for any particle
  if (m_BkgSensitiveDetector != nullptr) {
    m_BkgSensitiveDetector->step(step, history);
  }

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
    if (depth < m_DepthPlane) {
      B2WARNING("BKLM SensitiveDetector::step(): "
                << LogVar("Touchable HistoryDepth", depth)
                << LogVar("Should be at least", m_DepthPlane));
      return false;
    }
    int plane = hist->GetCopyNumber(depth - m_DepthPlane);
    int layer = hist->GetCopyNumber(depth - m_DepthLayer);
    int sector = hist->GetCopyNumber(depth - m_DepthSector);
    int section = hist->GetCopyNumber(depth - m_DepthSection);
    int moduleID =
      int(BKLMElementNumbers::moduleNumber(section, sector, layer));
    double time = 0.5 * (preStep->GetGlobalTime() + postStep->GetGlobalTime());  // GEANT4: in ns
    if (time > m_HitTimeMax)
      return false;
    const CLHEP::Hep3Vector globalPosition = 0.5 * (preStep->GetPosition() + postStep->GetPosition()) / CLHEP::cm; // in cm
    const Module* m = m_GeoPar->findModule(section, sector, layer);
    const CLHEP::Hep3Vector localPosition = m->globalToLocal(globalPosition);
    if (postStep->GetProcessDefinedStep() != 0) {
      if (postStep->GetProcessDefinedStep()->GetProcessType() == fDecay)
        moduleID |= BKLM_DECAYED_MASK;
    }
    int trackID = track->GetTrackID();
    if (m->hasRPCs()) {
      const CLHEP::Hep3Vector propagationTimes =
        m->getPropagationTimes(localPosition);
      int phiStripLower = -1;
      int phiStripUpper = -1;
      int zStripLower = -1;
      int zStripUpper = -1;
      convertHitToRPCStrips(localPosition, m, phiStripLower, phiStripUpper, zStripLower, zStripUpper);
      if (zStripLower > 0) {
        int moduleIDZ = moduleID;
        BKLMElementNumbers::setPlaneInModule(
          moduleIDZ, BKLMElementNumbers::c_ZPlane);
        BKLMElementNumbers::setStripInModule(moduleIDZ, zStripLower);
        BKLMStatus::setMaximalStrip(moduleIDZ, zStripUpper);
        BKLMSimHit* simHit = m_SimHits.appendNew(moduleIDZ, propagationTimes.z(), time, eDep);
        m_MCParticlesToSimHits.add(trackID, simHit->getArrayIndex());
        BKLMSimHitPosition* simHitPosition = m_SimHitPositions.appendNew(globalPosition.x(), globalPosition.y(), globalPosition.z());
        simHitPosition->addRelationTo(simHit);
      }
      if (phiStripLower > 0) {
        BKLMElementNumbers::setPlaneInModule(
          moduleID, BKLMElementNumbers::c_PhiPlane);
        BKLMElementNumbers::setStripInModule(moduleID, phiStripLower);
        BKLMStatus::setMaximalStrip(moduleID, phiStripUpper);
        BKLMSimHit* simHit = m_SimHits.appendNew(moduleID, propagationTimes.y(), time, eDep);
        m_MCParticlesToSimHits.add(trackID, simHit->getArrayIndex());
        BKLMSimHitPosition* simHitPosition = m_SimHitPositions.appendNew(globalPosition.x(), globalPosition.y(), globalPosition.z());
        simHitPosition->addRelationTo(simHit);
      }
    } else {
      int scint = hist->GetCopyNumber(depth - m_DepthScintillator);
      bool phiPlane = (plane == BKLM_INNER);
      double propagationTime =
        m->getPropagationTime(localPosition, scint, phiPlane);
      BKLMElementNumbers::setStripInModule(moduleID, scint);
      BKLMStatus::setMaximalStrip(moduleID, scint);
      if (phiPlane) {
        BKLMElementNumbers::setPlaneInModule(
          moduleID, BKLMElementNumbers::c_PhiPlane);
      } else {
        BKLMElementNumbers::setPlaneInModule(
          moduleID, BKLMElementNumbers::c_ZPlane);
      }
      BKLMSimHit* simHit = m_SimHits.appendNew(moduleID, propagationTime, time,
                                               eDep);
      m_MCParticlesToSimHits.add(trackID, simHit->getArrayIndex());
      BKLMSimHitPosition* simHitPosition = m_SimHitPositions.appendNew(globalPosition.x(), globalPosition.y(), globalPosition.z());
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
  if (phiStrip < pMin)
    return;
  int pMax = m->getPhiStripMax();
  if (phiStrip > pMax)
    return;

  double zStripD = m->getZStrip(localPosition);
  int zStrip = int(zStripD);
  int zMin = m->getZStripMin();
  if (zStrip < zMin)
    return;
  int zMax = m->getZStripMax();
  if (zStrip > zMax)
    return;

  phiStripLower = phiStrip;
  phiStripUpper = phiStrip;
  zStripLower = zStrip;
  zStripUpper = zStrip;
  double phiStripDiv = fmod(phiStripD, 1.0) - 0.5; // between -0.5 and +0.5 within central phiStrip
  double zStripDiv = fmod(zStripD, 1.0) - 0.5;   // between -0.5 and +0.5 within central zStrip
  int n = 0;
  double rand = gRandom->Uniform();
  for (n = 1; n < m_SimPar->getMaxMultiplicity(); ++n) {
    if (m_SimPar->getPhiMultiplicityCDF(phiStripDiv, n) > rand)
      break;
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
    if (m_SimPar->getZMultiplicityCDF(zStripDiv, n) > rand)
      break;
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
