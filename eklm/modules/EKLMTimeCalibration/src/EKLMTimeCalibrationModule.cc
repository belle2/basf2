/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* System headers. */
#include <fcntl.h>
#include <unistd.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/modules/EKLMTimeCalibration/EKLMTimeCalibrationModule.h>
#include <framework/core/ModuleManager.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>

using namespace Belle2;

REG_MODULE(EKLMTimeCalibration)

EKLMTimeCalibrationModule::EKLMTimeCalibrationModule() : Module()
{
  setDescription("Module for EKLM calibration.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("OutputFile", m_outputFileName, "Output file.",
           std::string("eklm_time_calibration.root"));
  m_outputFile = NULL;
  m_nStripDifferent = -1;
  m_Tree = NULL;
  m_ev = {0, 0};
}

EKLMTimeCalibrationModule::~EKLMTimeCalibrationModule()
{
}

void EKLMTimeCalibrationModule::initialize()
{
  int i;
  char str[128];
  TTree* t;
  StoreArray<EKLMHit2d>::required();
  StoreArray<EKLMDigit>::required();
  StoreArray<Track>::required();
  StoreArray<ExtHit>::required();
  if (m_geoDat.read() != 0)
    B2FATAL("Cannot read geometry data file.");
  EKLM::transformsToGlobal(&m_geoDat.transf);
  m_outputFile = new TFile(m_outputFileName.c_str(), "recreate");
  if (m_outputFile->IsZombie())
    B2FATAL("Cannot open output file.");
  m_nStripDifferent = m_geoDat.getNStripsDifferentLength();
  m_Tree = new TTree*[m_nStripDifferent];
  for (i = 0; i < m_nStripDifferent; i++) {
    snprintf(str, 128, "t%d", i);
    m_Tree[i] = new TTree(str, "");
    m_Tree[i]->Branch("time", &m_ev.time, "time/F");
    m_Tree[i]->Branch("dist", &m_ev.dist, "dist/F");
  }
  t = new TTree("t_strips", "");
  t->Branch("n", &m_nStripDifferent, "n/I");
  float len[m_nStripDifferent];
  for (i = 0; i < m_nStripDifferent; i++)
    len[i] = m_geoDat.getStripLength(m_geoDat.getStripPositionIndex(i) + 1);
  t->Branch("len", &len, "len[n]/F");
  t->Fill();
  m_outputFile->cd();
  t->Write();
  delete t;
}

void EKLMTimeCalibrationModule::beginRun()
{
}

void EKLMTimeCalibrationModule::event()
{
  int i, j, k, n, n2, vol;
  double l, hitTime;
  TVector3 hitPosition;
  HepGeom::Point3D<double> hitGlobal, hitLocal;
  StoreArray<Track> tracks;
  StoreArray<EKLMHit2d> hit2ds;
  std::multimap<int, ExtHit*> mapExtHit;
  std::multimap<int, ExtHit*>::iterator it, itLower, itUpper;
  ExtHit* extHit, *entryHit[2], *exitHit[2];
  HepGeom::Transform3D* tr;
  n = tracks.getEntries();
  for (i = 0; i < n; i++) {
    RelationVector<ExtHit> extHits = tracks[i]->getRelationsTo<ExtHit>();
    n2 = extHits.size();
    for (j = 0; j < n2; j++) {
      if (extHits[j]->getDetectorID() != Const::EDetector::KLM)
        continue;
      if (!m_geoDat.hitInEKLM(extHits[j]->getPosition().Z()))
        continue;
      mapExtHit.insert(std::pair<int, ExtHit*>(extHits[j]->getCopyID(),
                                               extHits[j]));
    }
  }
  n = hit2ds.getEntries();
  for (i = 0; i < n; i++) {
    RelationVector<EKLMDigit> digits = hit2ds[i]->getRelationsTo<EKLMDigit>();
    if (digits.size() != 2)
      B2FATAL("Wrong number of related EKLMDigits.");
    for (j = 0; j < 2; j++) {
      entryHit[j] = NULL;
      exitHit[j] = NULL;
      vol = digits[j]->getVolumeID();
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
      hitTime = 0.5 * (entryHit[j]->getTOF() + exitHit[j]->getTOF());
      hitPosition = 0.5 * (entryHit[j]->getPosition() +
                           exitHit[j]->getPosition());
      l = m_geoDat.getStripLength(digits[j]->getStrip());
      hitGlobal.setX(hitPosition.X() / Unit::mm * CLHEP::mm);
      hitGlobal.setY(hitPosition.Y() / Unit::mm * CLHEP::mm);
      hitGlobal.setZ(hitPosition.Z() / Unit::mm * CLHEP::mm);
      tr = getStripGlobalToLocal(&m_geoDat.transf, digits[j]);
      hitLocal = (*tr) * hitGlobal;
      m_ev.time = digits[j]->getTime() - hitTime;
      m_ev.dist = 0.5 * l - hitLocal.x() / CLHEP::mm * Unit::mm;
      k = m_geoDat.getStripLengthIndex(digits[j]->getStrip() - 1);
      m_Tree[k]->Fill();
    }
  }
}

void EKLMTimeCalibrationModule::endRun()
{
}

void EKLMTimeCalibrationModule::terminate()
{
  int i;
  m_outputFile->cd();
  for (i = 0; i < m_nStripDifferent; i++) {
    m_Tree[i]->Write();
    delete m_Tree[i];
  }
  delete[] m_Tree;
  delete m_outputFile;
}

