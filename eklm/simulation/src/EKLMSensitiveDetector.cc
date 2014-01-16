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
#include <G4Step.hh>

/* Belle2 headers. */
#include <eklm/geometry/EKLMObjectNumbers.h>
#include <eklm/simulation/EKLMSensitiveDetector.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLM::EKLMSensitiveDetector::
EKLMSensitiveDetector(G4String name, enum EKLMSensitiveType type)
: Simulation::SensitiveDetectorBase(name, Const::KLM)
{
  m_type = type;
  GearDir gd = GearDir("/Detector/DetectorComponent[@name=\"EKLM\"]/Content");
  m_mode = (enum EKLMDetectorMode)gd.getInt("Mode");
  gd.append("/SensitiveDetector");
  m_ThresholdEnergyDeposit =
    Unit::convertValue(gd.getDouble("EnergyDepositionThreshold"), "MeV");
  m_ThresholdHitTime =
    Unit::convertValue(gd.getDouble("HitTimeThreshold") , "ns");

  StoreArray<EKLMSimHit> simHits;
  StoreArray<MCParticle> particles;
  RelationArray particleToSimHits(particles, simHits);
  registerMCParticleRelation(particleToSimHits);

  StoreArray<EKLMSimHit>::registerPersistent();
  RelationArray::registerPersistent<MCParticle, EKLMSimHit>();
}

//-----------------------------------------------------
// Method invoked for every step in sensitive detector
//-----------------------------------------------------
bool EKLM::EKLMSensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
{
  HepGeom::Point3D<double> gpos, lpos;
  G4TouchableHandle hist = aStep->GetPreStepPoint()->
                           GetTouchableHandle();

  /**
   * Get deposited energy
   */
  const G4double eDep = aStep->GetTotalEnergyDeposit();

  /**
   * in normal opearation mode (m_mode=0)
   * ignore tracks with small energy deposition
   * use "<=" instead of "<" to drop hits from neutrinos etc unless eDepositionThreshold is non-negative
   * Background studies m_mode=1 accepts all tracks
   */
  if (eDep <= m_ThresholdEnergyDeposit && m_mode == EKLM_DETECTOR_NORMAL)
    return false;

  /**
   * get reference to the track
   */
  const G4Track& track = * aStep->GetTrack();

  /**
   * get time of hit
   */
  const G4double hitTime = track.GetGlobalTime();


  /**
  * drop hit if global time is nan or if it is  more than
  * hitTimeThreshold (to avoid nuclei fission signals)
  */
  if (isnan(hitTime)) {
    B2ERROR("EKLMSensitiveDetector: global time is nan");
    return false;
  }
  // No time cut for background studeis
  if (hitTime > m_ThresholdHitTime && m_mode == 0) {
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
  EKLMSimHit* hit = new(simHits.nextFreeAddress())EKLMSimHit();
  hit->setMomentum(CLHEP::HepLorentzVector(track.GetMomentum(),
                                           track.GetTotalEnergy()));
  hit->setTrackID(track.GetTrackID());
  hit->setParentTrackID(track.GetParentID());
  hit->setLocalPosition(lpos * Unit::mm);
  hit->setGlobalPosition(gpos * Unit::mm);
  hit->setEDep(eDep);
  hit->setPDG(track.GetDefinition()->GetPDGEncoding());
  hit->setTime(hitTime);
  /** Get information on mother volumes and store them to the hit. */
  switch (m_type) {
    case EKLM_SENSITIVE_STRIP:
      hit->setStrip(hist->GetVolume(2)->GetCopyNo());
      hit->setPlane(hist->GetVolume(3)->GetCopyNo());
      hit->setSector(hist->GetVolume(4)->GetCopyNo());
      hit->setLayer(hist->GetVolume(5)->GetCopyNo());
      hit->setEndcap(hist->GetVolume(6)->GetCopyNo());
      hit->setVolumeID(stripNumber(hit->getEndcap(), hit->getLayer(),
                                   hit->getSector(), hit->getPlane(),
                                   hit->getStrip()));
      break;
    case EKLM_SENSITIVE_SIPM:
      hit->setStrip(hist->GetVolume(1)->GetCopyNo());
      hit->setPlane(hist->GetVolume(2)->GetCopyNo());
      hit->setSector(hist->GetVolume(3)->GetCopyNo());
      hit->setLayer(hist->GetVolume(4)->GetCopyNo());
      hit->setEndcap(hist->GetVolume(5)->GetCopyNo());
      hit->setVolumeID(stripNumber(hit->getEndcap(), hit->getLayer(),
                                   hit->getSector(), hit->getPlane(),
                                   hit->getStrip()) + 100000);
      break;
    case EKLM_SENSITIVE_BOARD:
      int brd = hist->GetVolume(1)->GetCopyNo() - 1;
      hit->setStrip((brd - 1) % 5 + 1); /* Board number, not strip. */
      hit->setPlane((brd - 1) / 5 + 1);
      hit->setSector(hist->GetVolume(2)->GetCopyNo());
      hit->setLayer(hist->GetVolume(3)->GetCopyNo());
      hit->setEndcap(hist->GetVolume(4)->GetCopyNo());
      hit->setVolumeID(boardNumber(hit->getEndcap(), hit->getLayer(),
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

