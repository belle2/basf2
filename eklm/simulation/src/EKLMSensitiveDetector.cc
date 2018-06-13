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
#include <eklm/dbobjects/EKLMChannels.h>
#include <eklm/dbobjects/EKLMSimulationParameters.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/simulation/EKLMSensitiveDetector.h>
#include <framework/database/DBObjPtr.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLM::EKLMSensitiveDetector::
EKLMSensitiveDetector(G4String name)
  : Simulation::SensitiveDetectorBase(name, Const::KLM)
{
  int iEndcap, iLayer, iSector, iPlane, iStrip, strip, maxStrip;
  const EKLMChannelData* channelData;
  m_ChannelActive = NULL;
  m_GeoDat = &(EKLM::GeometryData::Instance());
  DBObjPtr<EKLMSimulationParameters> simPar;
  if (!simPar.isValid())
    B2FATAL("EKLM simulation parameters are not available.");
  m_ThresholdHitTime = simPar->getHitTimeThreshold();
  DBObjPtr<EKLMChannels> channels;
  if (!channels.isValid())
    B2FATAL("EKLM channel data are not available.");
  maxStrip = m_GeoDat->getMaximalStripGlobalNumber();
  m_ChannelActive = new bool[maxStrip];
  for (iEndcap = 1; iEndcap <= m_GeoDat->getNEndcaps(); iEndcap++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iEndcap);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
          for (iStrip = 1; iStrip <= m_GeoDat->getNStrips(); iStrip++) {
            strip = m_GeoDat->stripNumber(iEndcap, iLayer, iSector, iPlane,
                                          iStrip);
            channelData = channels->getChannelData(strip);
            if (channelData == NULL)
              B2FATAL("Incomplete EKLM channel data.");
            m_ChannelActive[strip - 1] = channelData->getActive();
          }
        }
      }
    }
  }
  StoreArray<MCParticle> particles;
  m_SimHits.registerInDataStore();
  particles.registerRelationTo(m_SimHits);
  RelationArray particleToSimHits(particles, m_SimHits);
  registerMCParticleRelation(particleToSimHits);
}

EKLM::EKLMSensitiveDetector::~EKLMSensitiveDetector()
{
  if (m_ChannelActive != NULL)
    delete[] m_ChannelActive;
}

bool EKLM::EKLMSensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
{
  const int stripLevel = 1;
  int endcap, layer, sector, plane, strip, stripGlobal;
  HepGeom::Point3D<double> gpos, lpos;
  G4TouchableHandle hist = aStep->GetPreStepPoint()->GetTouchableHandle();
  endcap = hist->GetVolume(stripLevel + 6)->GetCopyNo();
  layer = hist->GetVolume(stripLevel + 5)->GetCopyNo();
  sector = hist->GetVolume(stripLevel + 4)->GetCopyNo();
  plane = hist->GetVolume(stripLevel + 3)->GetCopyNo();
  strip = hist->GetVolume(stripLevel)->GetCopyNo();
  stripGlobal = m_GeoDat->stripNumber(endcap, layer, sector, plane, strip);
  if (!m_ChannelActive[stripGlobal - 1])
    return false;
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
  hit->setEDep(eDep);
  hit->setPDG(track.GetDefinition()->GetPDGEncoding());
  hit->setTime(hitTime);
  hit->setStrip(strip);
  hit->setPlane(plane);
  hit->setSector(sector);
  hit->setLayer(layer);
  hit->setEndcap(endcap);
  hit->setVolumeID(stripGlobal);
  /* Relation. */
  StoreArray<MCParticle> particles;
  RelationArray particleToSimHits(particles, m_SimHits);
  particleToSimHits.add(track.GetTrackID(), m_SimHits.getEntries() - 1);
  return true;
}

