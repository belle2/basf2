/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TF1.h>
#include <TCanvas.h>

/* Belle2 headers. */
#include <eklm/modules/EKLMAlignmentAlongStrips/EKLMAlignmentAlongStripsCollectorModule.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <tracking/dataobjects/ExtHit.h>

using namespace Belle2;

REG_MODULE(EKLMAlignmentAlongStripsCollector)

EKLMAlignmentAlongStripsCollectorModule::
EKLMAlignmentAlongStripsCollectorModule() : CalibrationCollectorModule()
{
  setDescription("Module for EKLM alignment along strip (data collection).");
  setPropertyFlags(c_ParallelProcessingCertified);
  m_Event = new EKLMAlignmentAlongStripsAlgorithm::Event;
  m_GeoDat = NULL;
  m_TransformData = NULL;
}

EKLMAlignmentAlongStripsCollectorModule::
~EKLMAlignmentAlongStripsCollectorModule()
{
  delete m_Event;
}

void EKLMAlignmentAlongStripsCollectorModule::prepare()
{
  TTree* t;
  m_GeoDat = &(EKLM::GeometryData::Instance());
  m_TransformData =
    new EKLM::TransformData(true, EKLM::TransformData::c_Alignment);
  m_EKLMDigits.isRequired();
  m_Tracks.isRequired();
  StoreArray<ExtHit> extHits;
  m_Tracks.requireRelationTo(extHits);
  t = new TTree("calibration_data", "");
  t->Branch("event", &m_Event);
  registerObject<TTree>("calibration_data", t);
}

void EKLMAlignmentAlongStripsCollectorModule::collect()
{
  int i, j, n, n2, vol;
  double l;
  const HepGeom::Transform3D* tr;
  TVector3 hitPosition;
  HepGeom::Point3D<double> hitGlobal, hitLocal;
  std::multimap<int, ExtHit*> mapExtHit;
  std::multimap<int, ExtHit*>::iterator it, it2, itLower, itUpper;
  std::set<int> digitVolumes;
  ExtHit* extHit;
  TTree* calibrationData = getObjectPtr<TTree>("calibration_data");
  /* Create volume - extHit map. */
  n = m_Tracks.getEntries();
  for (i = 0; i < n; i++) {
    RelationVector<ExtHit> extHits = m_Tracks[i]->getRelationsTo<ExtHit>();
    n2 = extHits.size();
    for (j = 0; j < n2; j++) {
      if (extHits[j]->getDetectorID() != Const::EDetector::EKLM)
        continue;
      if (!m_GeoDat->hitInEKLM(extHits[j]->getPosition().Z()))
        continue;
      mapExtHit.insert(std::pair<int, ExtHit*>(extHits[j]->getCopyID(),
                                               extHits[j]));
    }
  }
  /* Create set of strips with signal. */
  n = m_EKLMDigits.getEntries();
  for (i = 0; i < n; i++) {
    vol = m_GeoDat->stripNumber(
            m_EKLMDigits[i]->getEndcap(), m_EKLMDigits[i]->getLayer(),
            m_EKLMDigits[i]->getSector(), m_EKLMDigits[i]->getPlane(),
            m_EKLMDigits[i]->getStrip());
    digitVolumes.insert(vol);
  }
  /* Search for strips with extHits, but without signal. */
  for (it = mapExtHit.begin(); it != mapExtHit.end();) {
    itLower = it;
    do {
      ++it;
    } while ((it != mapExtHit.end()) && (it->first == itLower->first));
    itUpper = it;
    if (digitVolumes.find(itLower->first) != digitVolumes.end())
      continue;
    /* Found: write all extHits to the data tree. */
    for (it2 = itLower; it2 != itUpper; ++it2) {
      extHit = it2->second;
      hitPosition = extHit->getPosition();
      m_Event->x = hitPosition.X();
      m_Event->y = hitPosition.Y();
      m_Event->z = hitPosition.Z();
      hitGlobal.setX(hitPosition.X() / Unit::mm * CLHEP::mm);
      hitGlobal.setY(hitPosition.Y() / Unit::mm * CLHEP::mm);
      hitGlobal.setZ(hitPosition.Z() / Unit::mm * CLHEP::mm);
      m_Event->stripGlobal = it2->first;
      m_GeoDat->stripNumberToElementNumbers(
        m_Event->stripGlobal, &m_Event->endcap, &m_Event->layer,
        &m_Event->sector, &m_Event->plane, &m_Event->strip);
      tr = m_TransformData->getStripGlobalToLocal(
             m_Event->endcap, m_Event->layer, m_Event->sector, m_Event->plane,
             m_Event->strip);
      hitLocal = (*tr) * hitGlobal;
      l = m_GeoDat->getStripLength(m_Event->strip) / CLHEP::mm * Unit::mm;
      m_Event->distSiPM = 0.5 * l - hitLocal.x() / CLHEP::mm * Unit::mm;
      m_Event->distFarEnd = 0.5 * l + hitLocal.x() / CLHEP::mm * Unit::mm;
      m_Event->segmentGlobal =
        m_GeoDat->segmentNumber(
          m_Event->endcap, m_Event->layer, m_Event->sector, m_Event->plane,
          (m_Event->strip - 1) / m_GeoDat->getNStripsSegment() + 1);
      calibrationData->Fill();
    }
  }
}

void EKLMAlignmentAlongStripsCollectorModule::finish()
{
  delete m_TransformData;
}

