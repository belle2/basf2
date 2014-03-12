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
#include <framework/datastore/RelationArray.h>
#include <framework/dataobjects/EventMetaData.h>
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

BKLMReconstructorModule::BKLMReconstructorModule() : Module()
{
  setDescription("BKLM reconstruction module of 1D and 2D hits");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
  addParam("Coincidence window (ns)", m_dtMax,
           "Strip hits whose time difference exceeds this value are independent",
           double(75.0));
}

BKLMReconstructorModule::~BKLMReconstructorModule()
{
}

void BKLMReconstructorModule::initialize()
{
  StoreArray<BKLMDigit>::required();

  // Force creation and persistence of BKLM output datastores
  StoreArray<BKLMHit1d>::registerPersistent();
  StoreArray<BKLMHit2d>::registerPersistent();
  RelationArray::registerPersistent<BKLMHit1d, BKLMDigit>();
  RelationArray::registerPersistent<BKLMHit2d, BKLMHit1d>();

  m_GeoPar = Belle2::bklm::GeometryPar::instance();

}

void BKLMReconstructorModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  B2INFO("BKLMReconstructorModule::beginRun(): experiment " << evtMetaData->getExperiment() << "  run " << evtMetaData->getRun())
}

void BKLMReconstructorModule::event()
{

  // Construct StoreArray<BKLMHit1D> from StoreArray<BKLMDigit>

  StoreArray<BKLMDigit> digits;
  // sort by module+strip number
  std::map<int, int> volIDToDigits;
  for (int d = 0; d < digits.getEntries(); ++d) {
    BKLMDigit* digit = digits[d];
    volIDToDigits.insert(std::pair<int, int>(digit->getModuleID() + digit->getStrip(), d));
  }
  if (volIDToDigits.empty()) return;

  StoreArray<BKLMHit1d> hit1ds;
  RelationArray hit1dToDigits(hit1ds, digits);

  std::vector<unsigned int> indices;
  std::vector<BKLMDigit*> cluster;
  int oldVolID = volIDToDigits.begin()->first;
  double averageTime = digits[volIDToDigits.begin()->second]->getTime();

  for (std::map<int, int>::iterator iVolMap = volIDToDigits.begin(); iVolMap != volIDToDigits.end(); ++iVolMap) {
    int d = iVolMap->second;
    BKLMDigit* digit = digits[d];
    if ((iVolMap->first > oldVolID + 1) || (std::fabs(digit->getTime() - averageTime) > m_dtMax)) {
      new(hit1ds.nextFreeAddress()) BKLMHit1d(cluster);
      hit1dToDigits.add(hit1ds.getEntries() - 1, indices);
      indices.clear();
      cluster.clear();
    }
    double n = (double)(indices.size());
    averageTime = (n * averageTime + digit->getTime()) / (n + 1.0);
    indices.push_back(d);
    cluster.push_back(digit);
    oldVolID = iVolMap->first;
  }
  new(hit1ds.nextFreeAddress()) BKLMHit1d(cluster);
  hit1dToDigits.add(hit1ds.getEntries() - 1, indices);

  // Construct StoreArray<BKLMHit2D> from orthogonal same-module hits in StoreArray<BKLMHit1D>

  StoreArray<BKLMHit2d> hit2ds;
  RelationArray hit2dToHit1d(hit2ds, hit1ds);

  for (int i = 0; i < hit1ds.getEntries(); ++i) {
    int moduleID = hit1ds[i]->getModuleID() & ~MODULE_PLANE_MASK;
    bool isPhiReadout = hit1ds[i]->isPhiReadout();
    double time = hit1ds[i]->getTime();
    for (int j = i; j < hit1ds.getEntries(); ++j) {
      if (moduleID != (hit1ds[j]->getModuleID() & ~MODULE_PLANE_MASK)) continue;
      if (isPhiReadout == hit1ds[j]->isPhiReadout()) continue;
      if (std::fabs(time - hit1ds[j]->getTime()) > m_dtMax) continue;
      BKLMHit2d* hit2d = new(hit2ds.nextFreeAddress()) BKLMHit2d(hit1ds[i], hit1ds[j]);
      hit2dToHit1d.add(hit2ds.getEntries() - 1, i);
      hit2dToHit1d.add(hit2ds.getEntries() - 1, j);
      const bklm::Module* m = m_GeoPar->findModule(hit2d->isForward(), hit2d->getSector(), hit2d->getLayer());
      CLHEP::Hep3Vector local = m->getLocalPosition(hit2d->getPhiStripAve(), hit2d->getZStripAve());
      CLHEP::Hep3Vector global = m->localToGlobal(local);
      hit2d->setLocalPosition(local.x(), local.y(), local.z());
      hit2d->setGlobalPosition(global.x(), global.y(), global.z());
    }
  }

}

void BKLMReconstructorModule::endRun()
{
}

void BKLMReconstructorModule::terminate()
{
}
