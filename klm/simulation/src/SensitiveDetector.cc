/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/simulation/SensitiveDetector.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/eklm/geometry/GeometryData.h>

/* Basf2 headers. */
#include <simulation/background/BkgSensitiveDetector.h>

/* Geant4 headers. */
#include <G4Step.hh>
#include <G4VProcess.hh>

/* CLHEP headers. */
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Vector/ThreeVector.h>

/* ROOT headers. */
#include <TRandom3.h>

using namespace Belle2::KLM;

SensitiveDetector::SensitiveDetector(
  const G4String& name, KLMSubdetectorNumber subdetector) :
  SensitiveDetectorBase(name, Const::KLM),
  m_Subdetector(subdetector),
  m_FirstCall(true),
  m_BkgSensitiveDetector(nullptr),
  m_GeoPar(nullptr)
{
  if (!m_SimPar.isValid())
    B2FATAL("BKLM simulation parameters are not available.");
  m_HitTimeMax = m_SimPar->getHitTimeMax();
  m_MCParticles.isOptional();
  m_KLMSimHits.registerInDataStore();
  m_MCParticlesToKLMSimHits.registerInDataStore();
  registerMCParticleRelation(m_MCParticlesToKLMSimHits);
}

bool SensitiveDetector::stepEKLM(G4Step* aStep, G4TouchableHistory* history)
{
  /* Once-only initializations (constructor is called too early for these). */
  if (m_FirstCall) {
    m_FirstCall = false;
    const EKLM::GeometryData* geometryData = &EKLM::GeometryData::Instance();
    if (geometryData->beamBackgroundStudy())
      m_BkgSensitiveDetector = new BkgSensitiveDetector("EKLM");
  }

  // Record a BeamBackHit for any particle
  if (m_BkgSensitiveDetector != nullptr) {
    m_BkgSensitiveDetector->step(aStep, history);
  }

  const int stripLevel = 1;
  int section, layer, sector, plane, strip, stripGlobal;
  HepGeom::Point3D<double> gpos, lpos;
  G4TouchableHandle hist = aStep->GetPreStepPoint()->GetTouchableHandle();
  section = hist->GetVolume(stripLevel + 6)->GetCopyNo();
  layer = hist->GetVolume(stripLevel + 5)->GetCopyNo();
  sector = hist->GetVolume(stripLevel + 4)->GetCopyNo();
  plane = hist->GetVolume(stripLevel + 3)->GetCopyNo();
  strip = hist->GetVolume(stripLevel)->GetCopyNo();
  stripGlobal = m_ElementNumbers->stripNumber(
                  section, layer, sector, plane, strip);
  const G4double eDep = aStep->GetTotalEnergyDeposit();
  /* Do not record hits without deposited energy. */
  if (eDep <= 0)
    return false;
  const G4Track& track = * aStep->GetTrack();
  const G4double hitTime = track.GetGlobalTime();
  if (hitTime > m_HitTimeMax)
    return false;
  /* Hit position. */
  gpos = 0.5 * (aStep->GetPostStepPoint()->GetPosition() +
                aStep->GetPreStepPoint()->GetPosition());
  lpos = hist->GetHistory()->GetTopTransform().TransformPoint(gpos);
  /* Create step hit and store in to DataStore */
  KLMSimHit* hit = m_KLMSimHits.appendNew();
  hit->setSubdetector(KLMElementNumbers::c_EKLM);
  hit->setSection(section);
  hit->setSector(sector);
  hit->setLayer(layer);
  hit->setPlane(plane);
  hit->setStrip(strip);
  hit->setLastStrip(strip);
  // hit->setPropagationTime();
  hit->setTime(hitTime);
  hit->setEnergyDeposit(eDep);
  CLHEP::Hep3Vector trackMomentum = track.GetMomentum();
  hit->setMomentum(ROOT::Math::PxPyPzEVector(trackMomentum.x(), trackMomentum.y(),
                                             trackMomentum.z(), track.GetTotalEnergy()));
  hit->setTrackID(track.GetTrackID());
  hit->setParentTrackID(track.GetParentID());
  hit->setLocalPosition(lpos.x() / CLHEP::mm * Unit::mm,
                        lpos.y() / CLHEP::mm * Unit::mm,
                        lpos.z() / CLHEP::mm * Unit::mm);
  hit->setPosition(gpos.x() / CLHEP::mm * Unit::mm,
                   gpos.y() / CLHEP::mm * Unit::mm,
                   gpos.z() / CLHEP::mm * Unit::mm);
  hit->setPDG(track.GetDefinition()->GetPDGEncoding());
  hit->setVolumeID(stripGlobal);
  m_MCParticlesToKLMSimHits.add(track.GetTrackID(), hit->getArrayIndex());
  return true;
}

G4bool SensitiveDetector::stepBKLM(G4Step* step, G4TouchableHistory* history)
{
  /* Once-only initializations (constructor is called too early for these). */
  if (m_FirstCall) {
    m_FirstCall = false;
    m_GeoPar = bklm::GeometryPar::instance();
    if (m_GeoPar->doBeamBackgroundStudy())
      m_BkgSensitiveDetector = new BkgSensitiveDetector("BKLM");
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

  // Record a KLMSimHit for a charged track that deposits some energy.
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
    double time = 0.5 * (preStep->GetGlobalTime() + postStep->GetGlobalTime());  // GEANT4: in ns
    if (time > m_HitTimeMax)
      return false;
    const CLHEP::Hep3Vector globalPosition = 0.5 * (preStep->GetPosition() + postStep->GetPosition()) / CLHEP::cm; // in cm
    const bklm::Module* m = m_GeoPar->findModule(section, sector, layer);
    const CLHEP::Hep3Vector localPosition = m->globalToLocal(globalPosition);
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
        KLMSimHit* simHit = m_KLMSimHits.appendNew();
        simHit->setSubdetector(KLMElementNumbers::c_BKLM);
        simHit->setSection(section);
        simHit->setSector(sector);
        simHit->setLayer(layer);
        simHit->setPlane(BKLMElementNumbers::c_ZPlane);
        simHit->setStrip(zStripLower);
        simHit->setLastStrip(zStripUpper);
        simHit->setPropagationTime(propagationTimes.z());
        simHit->setTime(time);
        simHit->setEnergyDeposit(eDep);
        simHit->setPosition(globalPosition.x(), globalPosition.y(),
                            globalPosition.z());
        simHit->setPDG(track->GetDefinition()->GetPDGEncoding());
        m_MCParticlesToKLMSimHits.add(trackID, simHit->getArrayIndex());
      }
      if (phiStripLower > 0) {
        KLMSimHit* simHit = m_KLMSimHits.appendNew();
        simHit->setSubdetector(KLMElementNumbers::c_BKLM);
        simHit->setSection(section);
        simHit->setSector(sector);
        simHit->setLayer(layer);
        simHit->setPlane(BKLMElementNumbers::c_PhiPlane);
        simHit->setStrip(phiStripLower);
        simHit->setLastStrip(phiStripUpper);
        simHit->setPropagationTime(propagationTimes.y());
        simHit->setTime(time);
        simHit->setEnergyDeposit(eDep);
        simHit->setPosition(globalPosition.x(), globalPosition.y(),
                            globalPosition.z());
        simHit->setPDG(track->GetDefinition()->GetPDGEncoding());
        m_MCParticlesToKLMSimHits.add(trackID, simHit->getArrayIndex());
      }
    } else {
      int scint = hist->GetCopyNumber(depth - m_DepthScintillator);
      bool phiPlane = (plane == BKLM_INNER);
      double propagationTime =
        m->getPropagationTime(localPosition, scint, phiPlane);
      KLMSimHit* simHit = m_KLMSimHits.appendNew();
      simHit->setSubdetector(KLMElementNumbers::c_BKLM);
      simHit->setSection(section);
      simHit->setSector(sector);
      simHit->setLayer(layer);
      if (phiPlane) {
        simHit->setPlane(BKLMElementNumbers::c_PhiPlane);
      } else {
        simHit->setPlane(BKLMElementNumbers::c_ZPlane);
      }
      simHit->setStrip(scint);
      simHit->setLastStrip(scint);
      simHit->setPropagationTime(propagationTime);
      simHit->setTime(time);
      simHit->setEnergyDeposit(eDep);
      simHit->setPosition(globalPosition.x(), globalPosition.y(),
                          globalPosition.z());
      simHit->setPDG(track->GetDefinition()->GetPDGEncoding());
      m_MCParticlesToKLMSimHits.add(trackID, simHit->getArrayIndex());
    }
    return true;
  }
  return false;
}

void SensitiveDetector::convertHitToRPCStrips(
  const CLHEP::Hep3Vector& localPosition, const bklm::Module* m,
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
      phiStripLower = std::min(phiStrip, phiStripLower);
      phiStripUpper = std::max(phiStrip, phiStripUpper);
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
      zStripLower = std::min(zStrip, zStripLower);
      zStripUpper = std::max(zStrip, zStripUpper);
    }
    nextStrip = (nextStrip > 0 ? -(1 + nextStrip) : 1 - nextStrip);
  }
  return;
}

bool SensitiveDetector::step(G4Step* step, G4TouchableHistory* history)
{
  if (m_Subdetector == KLMElementNumbers::c_BKLM)
    return stepBKLM(step, history);
  else
    return stepEKLM(step, history);
}
