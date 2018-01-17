/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
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
#include <eklm/modules/EKLMTimeCalibration/EKLMTimeCalibrationCollectorModule.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <tracking/dataobjects/ExtHit.h>

using namespace Belle2;

REG_MODULE(EKLMTimeCalibrationCollector)

EKLMTimeCalibrationCollectorModule::EKLMTimeCalibrationCollectorModule() :
  CalibrationCollectorModule()
{
  setDescription("Module for EKLM time calibration (data collection).");
  setPropertyFlags(c_ParallelProcessingCertified);
  m_ev = {0, 0, 0};
  m_Strip = 0;
  m_TransformData = NULL;
  m_GeoDat = NULL;
}

EKLMTimeCalibrationCollectorModule::~EKLMTimeCalibrationCollectorModule()
{
}

void EKLMTimeCalibrationCollectorModule::prepare()
{
  TTree* t;
  m_GeoDat = &(EKLM::GeometryData::Instance());
  m_EKLMHit2ds.isRequired();
  m_Tracks.isRequired();
  StoreArray<EKLMDigit> eklmDigits;
  m_EKLMHit2ds.requireRelationTo(eklmDigits);
  StoreArray<ExtHit> extHits;
  m_Tracks.requireRelationTo(extHits);
  m_EventT0.isRequired("EventT0");
  m_TransformData = new EKLM::TransformData(true, EKLM::TransformData::c_None);
  t = new TTree("calibration_data", "");
  t->Branch("time", &m_ev.time, "time/F");
  t->Branch("dist", &m_ev.dist, "dist/F");
  t->Branch("npe", &m_ev.npe, "npe/F");
  t->Branch("strip", &m_Strip, "strip/I");
  registerObject<TTree>("calibration_data", t);
}

void EKLMTimeCalibrationCollectorModule::collect()
{
  int i, j, n, n2, vol;
  double l, hitTime;
  TVector3 hitPosition;
  HepGeom::Point3D<double> hitGlobal, hitLocal;
  std::multimap<int, ExtHit*> mapExtHit;
  std::multimap<int, ExtHit*>::iterator it, itLower, itUpper;
  ExtHit* extHit, *entryHit[2], *exitHit[2];
  const HepGeom::Transform3D* tr;
  TTree* calibrationData = getObjectPtr<TTree>("calibration_data");
  n = m_Tracks.getEntries();
  if (!m_EventT0->hasEventT0()) {
    B2ERROR("Event T0 is not determined. "
            "Cannot collect data for EKLM time calibration.");
    return;
  }
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
  n = m_EKLMHit2ds.getEntries();
  for (i = 0; i < n; i++) {
    RelationVector<EKLMDigit> digits =
      m_EKLMHit2ds[i]->getRelationsTo<EKLMDigit>();
    if (digits.size() != 2)
      B2FATAL("Wrong number of related EKLMDigits.");
    /*
     * This is possible if the threshold was crossed, but the pedestal level
     * has been estimated incorrectly.
     */
    if (digits[0]->getNPE() == 0 || digits[1]->getNPE() == 0)
      continue;
    for (j = 0; j < 2; j++) {
      entryHit[j] = NULL;
      exitHit[j] = NULL;
      vol = m_GeoDat->stripNumber(digits[j]->getEndcap(), digits[j]->getLayer(),
                                  digits[j]->getSector(), digits[j]->getPlane(),
                                  digits[j]->getStrip());
      itLower = mapExtHit.lower_bound(vol);
      itUpper = mapExtHit.upper_bound(vol);
      for (it = itLower; it != itUpper; ++it) {
        extHit = it->second;
        switch (extHit->getStatus()) {
          case EXT_ENTER:
            if (entryHit[j] == NULL) {
              entryHit[j] = extHit;
            } else {
              if (extHit->getTOF() < entryHit[j]->getTOF())
                entryHit[j] = extHit;
            }
            break;
          case EXT_EXIT:
            if (exitHit[j] == NULL) {
              exitHit[j] = extHit;
            } else {
              if (extHit->getTOF() > exitHit[j]->getTOF())
                exitHit[j] = extHit;
            }
            break;
          default:
            break;
        }
      }
    }
    if (entryHit[0] == NULL || exitHit[0] == NULL ||
        entryHit[1] == NULL || exitHit[1] == NULL)
      continue;
    for (j = 0; j < 2; j++) {
      hitTime = 0.5 * (entryHit[j]->getTOF() + exitHit[j]->getTOF()) +
                m_EventT0->getEventT0();
      hitPosition = 0.5 * (entryHit[j]->getPosition() +
                           exitHit[j]->getPosition());
      l = m_GeoDat->getStripLength(digits[j]->getStrip()) / CLHEP::mm *
          Unit::mm;
      hitGlobal.setX(hitPosition.X() / Unit::mm * CLHEP::mm);
      hitGlobal.setY(hitPosition.Y() / Unit::mm * CLHEP::mm);
      hitGlobal.setZ(hitPosition.Z() / Unit::mm * CLHEP::mm);
      tr = m_TransformData->getStripGlobalToLocal(digits[j]);
      hitLocal = (*tr) * hitGlobal;
      m_ev.time = digits[j]->getTime() - hitTime;
      m_ev.dist = 0.5 * l - hitLocal.x() / CLHEP::mm * Unit::mm;
      m_ev.npe = digits[j]->getNPE();
      m_Strip =
        m_GeoDat->stripNumber(digits[j]->getEndcap(), digits[j]->getLayer(),
                              digits[j]->getSector(), digits[j]->getPlane(),
                              digits[j]->getStrip());
      calibrationData->Fill();
    }
  }
}

void EKLMTimeCalibrationCollectorModule::finish()
{
  if (m_TransformData != NULL)
    delete m_TransformData;
}

