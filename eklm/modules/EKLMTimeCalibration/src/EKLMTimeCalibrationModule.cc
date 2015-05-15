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
}

EKLMTimeCalibrationModule::~EKLMTimeCalibrationModule()
{
}

void EKLMTimeCalibrationModule::initialize()
{
  StoreArray<EKLMHit2d>::required();
  StoreArray<EKLMDigit>::required();
  StoreArray<Track>::required();
  StoreArray<ExtHit>::required();
  m_outputFile = new TFile(m_outputFileName.c_str(), "recreate");
  if (m_outputFile->IsZombie())
    B2FATAL("Cannot open output file.");
}

void EKLMTimeCalibrationModule::beginRun()
{
}

void EKLMTimeCalibrationModule::event()
{
  int i, j, n, n2, vol;
  bool entryFound, exitFound;
  double hitTime;
  TVector3 hitGlobalPosition;
  StoreArray<Track> tracks;
  StoreArray<EKLMHit2d> hit2ds;
  std::multimap<int, ExtHit*> mapExtHit;
  std::multimap<int, ExtHit*>::iterator it, itLower, itUpper;
  ExtHit* extHit, *entryHit, *exitHit;
  n = tracks.getEntries();
  for (i = 0; i < n; i++) {
    RelationVector<ExtHit> extHits = tracks[i]->getRelationsTo<ExtHit>();
    n2 = extHits.size();
    for (j = 0; j < n2; j++) {
      if (extHits[j]->getDetectorID() != Const::EDetector::KLM)
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
      vol = digits[j]->getVolumeID();
      itLower = mapExtHit.lower_bound(vol);
      itUpper = mapExtHit.upper_bound(vol);
      entryFound = false;
      exitFound = false;
      for (it = itLower; it != itUpper; ++it) {
        extHit = it->second;
        switch (extHit->getStatus()) {
          case EXT_ENTER:
            if (!entryFound) {
              entryFound = true;
              entryHit = extHit;
            } else {
              if (extHit->getTOF() < entryHit->getTOF())
                entryHit = extHit;
            }
            break;
          case EXT_EXIT:
            if (!exitFound) {
              exitFound = true;
              exitHit = extHit;
            } else {
              if (extHit->getTOF() > exitHit->getTOF())
                exitHit = extHit;
            }
            break;
          default:
            break;
        }
      }
      if (!(entryFound && exitFound))
        continue;
      hitTime = 0.5 * (entryHit->getTOF() + exitHit->getTOF());
      hitGlobalPosition = 0.5 * (entryHit->getPosition() +
                                 exitHit->getPosition());
      printf("Hit: %g %g %g %g\n", hitTime, hitGlobalPosition.X(),
             hitGlobalPosition.Y(), hitGlobalPosition.Z());
    }
  }
}

void EKLMTimeCalibrationModule::endRun()
{
}

void EKLMTimeCalibrationModule::terminate()
{
  delete m_outputFile;
}

