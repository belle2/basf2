/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <CLHEP/Geometry/Point3D.h>
#include <G4Step.hh>

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMSimulationParameters.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/simulation/EKLMSensitiveDetector.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLM::EKLMSensitiveDetector::
EKLMSensitiveDetector(G4String name, enum SensitiveType type)
  : Simulation::SensitiveDetectorBase(name, Const::KLM)
{
  DBObjPtr<EKLMSimulationParameters> simPar;
  if (!simPar.isValid())
    B2FATAL("EKLM simulation parameters are not available.");
  m_GeoDat = &(EKLM::GeometryData::Instance());
  m_type = type;
  m_ThresholdHitTime = simPar->getHitTimeThreshold();
  StoreArray<EKLMSimHit> simHits;
  StoreArray<MCParticle> particles;
  simHits.registerInDataStore();
  particles.registerRelationTo(simHits);
  RelationArray particleToSimHits(particles, simHits);
  registerMCParticleRelation(particleToSimHits);
}

bool EKLM::EKLMSensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
{
  int stripLevel = 1;
  HepGeom::Point3D<double> gpos, lpos;
  G4TouchableHandle hist = aStep->GetPreStepPoint()->GetTouchableHandle();
  const G4double eDep = aStep->GetTotalEnergyDeposit();
  const G4Track& track = * aStep->GetTrack();
  const G4double hitTime = track.GetGlobalTime();
  /* No time cut for background studies. */
  if (hitTime > m_ThresholdHitTime &&
      m_GeoDat->getDetectorMode() == EKLMGeometry::c_DetectorNormal) {
    B2INFO("EKLMSensitiveDetector: "
           " ALL HITS WITH TIME > hitTimeThreshold ARE DROPPED!!");
    return false;
  }
  /* Hit position. */
  gpos = 0.5 * (aStep->GetPostStepPoint()->GetPosition() +
                aStep->GetPreStepPoint()->GetPosition());
  lpos = hist->GetHistory()->GetTopTransform().TransformPoint(gpos);
  /* Create step hit and store in to DataStore */
  StoreArray<EKLMSimHit> simHits;
  EKLMSimHit* hit = simHits.appendNew();
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
  hit->setEDep(eDep);
  hit->setPDG(track.GetDefinition()->GetPDGEncoding());
  hit->setTime(hitTime);
  if (m_GeoDat->getDetectorMode() == EKLMGeometry::c_DetectorBackground)
    stripLevel = 2;
  /* Get information on mother volumes and store them to the hit. */
  switch (m_type) {
    case c_SensitiveStrip:
      hit->setStrip(hist->GetVolume(stripLevel)->GetCopyNo());
      hit->setPlane(hist->GetVolume(stripLevel + 3)->GetCopyNo());
      hit->setSector(hist->GetVolume(stripLevel + 4)->GetCopyNo());
      hit->setLayer(hist->GetVolume(stripLevel + 5)->GetCopyNo());
      hit->setEndcap(hist->GetVolume(stripLevel + 6)->GetCopyNo());
      hit->setVolumeID(m_GeoDat->stripNumber(hit->getEndcap(), hit->getLayer(),
                                             hit->getSector(), hit->getPlane(),
                                             hit->getStrip()));
      break;
    case c_SensitiveSiPM:
      hit->setStrip(hist->GetVolume(1)->GetCopyNo());
      hit->setPlane(hist->GetVolume(4)->GetCopyNo());
      hit->setSector(hist->GetVolume(5)->GetCopyNo());
      hit->setLayer(hist->GetVolume(6)->GetCopyNo());
      hit->setEndcap(hist->GetVolume(7)->GetCopyNo());
      hit->setVolumeID(m_GeoDat->stripNumber(hit->getEndcap(), hit->getLayer(),
                                             hit->getSector(), hit->getPlane(),
                                             hit->getStrip()) + 100000);
      break;
    case c_SensitiveBoard:
      int brd = hist->GetVolume(1)->GetCopyNo() - 1;
      hit->setStrip((brd - 1) % 5 + 1); /* Board number, not strip. */
      hit->setPlane((brd - 1) / 5 + 1);
      hit->setSector(hist->GetVolume(2)->GetCopyNo());
      hit->setLayer(hist->GetVolume(3)->GetCopyNo());
      hit->setEndcap(hist->GetVolume(4)->GetCopyNo());
      hit->setVolumeID(
        m_GeoDat->segmentNumber(hit->getEndcap(), hit->getLayer(),
                                hit->getSector(), hit->getPlane(),
                                hit->getStrip()) + 200000);
      break;
  }
  /* Relation. */
  StoreArray<MCParticle> particles;
  RelationArray particleToSimHits(particles, simHits);
  particleToSimHits.add(track.GetTrackID(), simHits.getEntries() - 1);
  return true;
}

