/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/modules/bklmReconstruction/BKLMReconstructorModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>

#include <bklm/geometry/GeometryPar.h>
#include <bklm/geometry/Module.h>
#include <bklm/dataobjects/BKLMDigit.h>
#include <bklm/dataobjects/BKLMHit1d.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <bklm/dataobjects/BKLMStatus.h>

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Matrix/Matrix.h"

using namespace std;
using namespace Belle2;
using namespace Belle2::bklm;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(BKLMReconstructor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BKLMReconstructorModule::BKLMReconstructorModule() : Module(), m_GeoPar(NULL)
{
  setDescription("BKLM reconstruction module of 1D and 2D hits");
  setPropertyFlags(c_ParallelProcessingCertified);
  // MC 1 GeV/c muons: 1-sigma width is 0.43 ns
  addParam("Orthogonal-strip coincidence window (ns)", m_DtMax,
           "Orthogonal strip hits whose time difference exceeds this value are independent",
           double(50.0));
  // MC 1 GeV/c muons: mean prompt time is 0.43 ns
  addParam("Nominal prompt time (ns)", m_PromptTime,
           "Nominal time of prompt hits",
           double(0.0));
  // MC 1 GeV/c muons: 1-sigma width is 0.15 ns
  addParam("Prompt window (ns)", m_PromptWindow,
           "Half-width of prompt window relative to PromptTime",
           double(50.0));
}

BKLMReconstructorModule::~BKLMReconstructorModule()
{
}

void BKLMReconstructorModule::initialize()
{
  StoreArray<BKLMDigit>::required();

  // Force creation and persistence of BKLM output datastores and relations
  StoreArray<BKLMHit1d> hit1ds;
  hit1ds.registerInDataStore();
  StoreArray<BKLMHit2d> hit2ds;
  hit2ds.registerInDataStore();
  StoreArray<BKLMDigit> digits;
  hit1ds.registerRelationTo(digits);
  hit2ds.registerRelationTo(hit1ds);

  m_GeoPar = Belle2::bklm::GeometryPar::instance();

}

void BKLMReconstructorModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  B2INFO("BKLMReconstructor: Experiment " << evtMetaData->getExperiment() << "  run " << evtMetaData->getRun())
}

void BKLMReconstructorModule::event()
{

  // Construct StoreArray<BKLMHit1D> from StoreArray<BKLMDigit>

  StoreArray<BKLMDigit> digits;
  // sort by module+strip number
  std::map<int, int> volIDToDigits;
  for (int d = 0; d < digits.getEntries(); ++d) {
    BKLMDigit* digit = digits[d];
    volIDToDigits.insert(std::pair<int, int>(digit->getModuleID() & BKLM_MODULESTRIPID_MASK, d));
  }
  if (volIDToDigits.empty()) return;

  StoreArray<BKLMHit1d> hit1ds;

  std::vector<BKLMDigit*> cluster;
  int oldVolID = volIDToDigits.begin()->first;
  double averageTime = digits[volIDToDigits.begin()->second]->getTime();

  for (std::map<int, int>::iterator iVolMap = volIDToDigits.begin(); iVolMap != volIDToDigits.end(); ++iVolMap) {
    BKLMDigit* digit = digits[iVolMap->second];
    if ((iVolMap->first > oldVolID + 1) || (std::fabs(digit->getTime() - averageTime) > m_DtMax)) {
      hit1ds.appendNew(cluster); // also creates relation hit1d to each digit in cluster
      cluster.clear();
    }
    double n = (double)(cluster.size());
    averageTime = (n * averageTime + digit->getTime()) / (n + 1.0);
    cluster.push_back(digit);
    oldVolID = iVolMap->first;
  }
  hit1ds.appendNew(cluster); // also creates relation hit1d to each digit in cluster

  // Construct StoreArray<BKLMHit2D> from orthogonal same-module hits in StoreArray<BKLMHit1D>

  StoreArray<BKLMHit2d> hit2ds;

  for (int i = 0; i < hit1ds.getEntries(); ++i) {
    int moduleID = hit1ds[i]->getModuleID() & BKLM_MODULEID_MASK;
    const bklm::Module* m = m_GeoPar->findModule(hit1ds[i]->isForward(), hit1ds[i]->getSector(), hit1ds[i]->getLayer());
    bool isPhiReadout = hit1ds[i]->isPhiReadout();
    for (int j = i + 1; j < hit1ds.getEntries(); ++j) {
      if (moduleID != (hit1ds[j]->getModuleID() & BKLM_MODULEID_MASK)) continue;
      if (isPhiReadout == hit1ds[j]->isPhiReadout()) continue;
      int phiIndex = isPhiReadout ? i : j;
      int zIndex   = isPhiReadout ? j : i;
      CLHEP::Hep3Vector local = m->getLocalPosition(hit1ds[phiIndex]->getStripAve(), hit1ds[zIndex]->getStripAve());
      CLHEP::Hep3Vector propagationTimes = m->getPropagationTimes(local);
      double phiTime = hit1ds[phiIndex]->getTime() - propagationTimes.y();
      double zTime = hit1ds[zIndex]->getTime() - propagationTimes.z();
      if (std::fabs(phiTime - zTime) > m_DtMax) continue;
      CLHEP::Hep3Vector global = m->localToGlobal(local);
      double time = 0.5 * (phiTime + zTime) - global.mag() / Const::speedOfLight;
      BKLMHit2d* hit2d = hit2ds.appendNew(hit1ds[phiIndex], hit1ds[zIndex], global, time); // also creates relations hit2d to each hit1d
      if (fabs(time - m_PromptTime) > m_PromptWindow) hit2d->isOutOfTime();
    }
  }

}

void BKLMReconstructorModule::endRun()
{
}

void BKLMReconstructorModule::terminate()
{
}
