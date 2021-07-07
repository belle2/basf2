/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/simulation/EKLMSensitiveDetector.h>

/* KLM headers. */
#include <klm/dbobjects/eklm/EKLMSimulationParameters.h>

/* Belle 2 headers. */
#include <framework/database/DBObjPtr.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

/* Geant4 headers. */
#include <G4Step.hh>

/* CLHEP headers. */
#include <CLHEP/Geometry/Point3D.h>

using namespace Belle2;

EKLM::EKLMSensitiveDetector::EKLMSensitiveDetector(G4String name) :
  Simulation::SensitiveDetectorBase(name, Const::KLM),
  m_ElementNumbers(&(EKLMElementNumbers::Instance()))
{
  DBObjPtr<EKLMSimulationParameters> simPar;
  if (!simPar.isValid())
    B2FATAL("EKLM simulation parameters are not available.");
  m_ThresholdHitTime = simPar->getHitTimeThreshold();
  m_MCParticles.isOptional();
  m_SimHits.registerInDataStore();
  m_MCParticles.registerRelationTo(m_SimHits);
  registerMCParticleRelation(m_MCParticlesToSimHits);
}

EKLM::EKLMSensitiveDetector::~EKLMSensitiveDetector()
{
}

bool EKLM::EKLMSensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
{
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
  /* No time cut for background studies. */
  if (hitTime > m_ThresholdHitTime) {
    B2INFO("EKLMSensitiveDetector: "
           " ALL HITS WITH TIME > hitTimeThreshold ARE DROPPED!!");
    return false;
  }
  /* Hit position. */
  gpos = 0.5 * (aStep->GetPostStepPoint()->GetPosition() +
                aStep->GetPreStepPoint()->GetPosition());
  lpos = hist->GetHistory()->GetTopTransform().TransformPoint(gpos);
  /* Create step hit and store in to DataStore */
  EKLMSimHit* hit = m_SimHits.appendNew();
  CLHEP::Hep3Vector trackMomentum = track.GetMomentum();
  hit->setMomentum(TLorentzVector(trackMomentum.x(), trackMomentum.y(),
                                  trackMomentum.z(), track.GetTotalEnergy()));
  hit->setTrackID(track.GetTrackID());
  hit->setParentTrackID(track.GetParentID());
  hit->setLocalPosition(lpos.x() / CLHEP::mm * Unit::mm,
                        lpos.y() / CLHEP::mm * Unit::mm,
                        lpos.z() / CLHEP::mm * Unit::mm);
  hit->setPosition(gpos.x() / CLHEP::mm * Unit::mm,
                   gpos.y() / CLHEP::mm * Unit::mm,
                   gpos.z() / CLHEP::mm * Unit::mm);
  hit->setEnergyDeposit(eDep);
  hit->setPDG(track.GetDefinition()->GetPDGEncoding());
  hit->setTime(hitTime);
  hit->setStrip(strip);
  hit->setPlane(plane);
  hit->setSector(sector);
  hit->setLayer(layer);
  hit->setSection(section);
  hit->setVolumeID(stripGlobal);
  m_MCParticlesToSimHits.add(track.GetTrackID(), hit->getArrayIndex());
  return true;
}

