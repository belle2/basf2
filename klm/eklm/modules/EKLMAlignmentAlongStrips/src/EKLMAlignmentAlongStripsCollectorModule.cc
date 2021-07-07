/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/modules/EKLMAlignmentAlongStrips/EKLMAlignmentAlongStripsCollectorModule.h>

/* Belle 2 headers. */
#include <framework/gearbox/Unit.h>
#include <tracking/dataobjects/ExtHit.h>

/* ROOT headers. */
#include <TTree.h>

using namespace Belle2;

REG_MODULE(EKLMAlignmentAlongStripsCollector)

EKLMAlignmentAlongStripsCollectorModule::
EKLMAlignmentAlongStripsCollectorModule() :
  CalibrationCollectorModule(),
  m_ElementNumbers(&(EKLMElementNumbers::Instance())),
  m_GeoDat(nullptr),
  m_TransformData(nullptr),
  m_Event(new EKLMAlignmentAlongStripsAlgorithm::Event)
{
  setDescription("Module for EKLM alignment along strip (data collection).");
  setPropertyFlags(c_ParallelProcessingCertified);
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
  m_KLMDigits.isRequired();
  m_Tracks.isRequired();
  StoreArray<ExtHit> extHits;
  m_Tracks.requireRelationTo(extHits);
  t = new TTree("calibration_data", "");
  t->Branch("event", &m_Event);
  registerObject<TTree>("calibration_data", t);
}

void EKLMAlignmentAlongStripsCollectorModule::collect()
{
  /* cppcheck-suppress variableScope */
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
  n = m_KLMDigits.getEntries();
  for (i = 0; i < n; i++) {
    if (m_KLMDigits[i]->getSubdetector() != KLMElementNumbers::c_EKLM)
      continue;
    vol = m_ElementNumbers->stripNumber(
            m_KLMDigits[i]->getSection(), m_KLMDigits[i]->getLayer(),
            m_KLMDigits[i]->getSector(), m_KLMDigits[i]->getPlane(),
            m_KLMDigits[i]->getStrip());
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
      m_ElementNumbers->stripNumberToElementNumbers(
        m_Event->stripGlobal, &m_Event->section, &m_Event->layer,
        &m_Event->sector, &m_Event->plane, &m_Event->strip);
      tr = m_TransformData->getStripGlobalToLocal(
             m_Event->section, m_Event->layer, m_Event->sector, m_Event->plane,
             m_Event->strip);
      hitLocal = (*tr) * hitGlobal;
      l = m_GeoDat->getStripLength(m_Event->strip) / CLHEP::mm * Unit::mm;
      m_Event->distSiPM = 0.5 * l - hitLocal.x() / CLHEP::mm * Unit::mm;
      m_Event->distFarEnd = 0.5 * l + hitLocal.x() / CLHEP::mm * Unit::mm;
      m_Event->segmentGlobal =
        m_ElementNumbers->segmentNumber(
          m_Event->section, m_Event->layer, m_Event->sector, m_Event->plane,
          (m_Event->strip - 1) / m_ElementNumbers->getNStripsSegment() + 1);
      calibrationData->Fill();
    }
  }
}

void EKLMAlignmentAlongStripsCollectorModule::finish()
{
  delete m_TransformData;
}

