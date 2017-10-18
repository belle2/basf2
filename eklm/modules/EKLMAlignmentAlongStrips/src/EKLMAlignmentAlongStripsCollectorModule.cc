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
#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/modules/EKLMAlignmentAlongStrips/EKLMAlignmentAlongStripsCollectorModule.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <mdst/dataobjects/Track.h>
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
  StoreArray<EKLMHit2d>::required();
  StoreArray<EKLMDigit>::required();
  StoreArray<Track>::required();
  StoreArray<ExtHit>::required();
  t = new TTree("calibration_data", "");
  t->Branch("event", &m_Event);
  registerObject<TTree>("calibration_data", t);
}

void EKLMAlignmentAlongStripsCollectorModule::collect()
{
  int i, j, n, n2, vol;
  TVector3 hitPosition;
  HepGeom::Point3D<double> hitGlobal, hitLocal;
  StoreArray<Track> tracks;
  StoreArray<EKLMDigit> digits;
  std::multimap<int, ExtHit*> mapExtHit;
  std::multimap<int, ExtHit*>::iterator it, it2, itLower, itUpper;
  std::set<int> digitVolumes;
  ExtHit* extHit;
  /* Create volume - extHit map. */
  n = tracks.getEntries();
  for (i = 0; i < n; i++) {
    RelationVector<ExtHit> extHits = tracks[i]->getRelationsTo<ExtHit>();
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
  n = digits.getEntries();
  for (i = 0; i < n; i++) {
    vol = m_GeoDat->stripNumber(digits[i]->getEndcap(), digits[i]->getLayer(),
                                digits[i]->getSector(), digits[i]->getPlane(),
                                digits[i]->getStrip());
    digitVolumes.insert(vol);
  }
  /* Search for strips with extHits, but without signal. */
  for (it = mapExtHit.begin(); it != mapExtHit.end();) {
    itLower = it;
    do {
      ++it;
    } while ((it != mapExtHit.end()) && (it->first == itLower->first));
    itUpper = it;
    if (digitVolumes.find(itLower->first) == digitVolumes.end())
      continue;
    /* Found: write all extHits to the data tree. */
    for (it2 = itLower; it2 != itUpper; ++it2) {
      extHit = it2->second;
      hitPosition = extHit->getPosition();
      m_Event->x = hitPosition.X();
      m_Event->y = hitPosition.Y();
      m_Event->z = hitPosition.Z();
      m_Event->stripGlobal = it2->first;
      m_GeoDat->stripNumberToElementNumbers(
        m_Event->stripGlobal, &m_Event->endcap, &m_Event->layer,
        &m_Event->sector, &m_Event->plane, &m_Event->strip);
      m_Event->segmentGlobal =
        m_GeoDat->segmentNumber(
          m_Event->endcap, m_Event->layer, m_Event->sector, m_Event->plane,
          (m_Event->strip - 1) / m_GeoDat->getNStripsSegment() + 1);
      getObject<TTree>("calibration_data").Fill();
    }
  }
}

void EKLMAlignmentAlongStripsCollectorModule::terminate()
{
}

