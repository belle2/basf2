/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/modules/bklmDigitizer/BKLMDigitizerModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <bklm/dataobjects/BKLMSimHit.h>
#include <bklm/dataobjects/BKLMDigit.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BKLMDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BKLMDigitizerModule::BKLMDigitizerModule() : Module()
{

  setDescription("Creates BKLMDigits from BKLMSimHits");
  setPropertyFlags(c_ParallelProcessingCertified);
}

BKLMDigitizerModule::~BKLMDigitizerModule()
{
}

void BKLMDigitizerModule::initialize()
{
  StoreArray<BKLMSimHit>::required();

  // Force creation of BKLM datastores
  StoreArray<BKLMDigit>::registerPersistent();
  RelationArray::registerPersistent<BKLMSimHit, BKLMDigit>();
}

void BKLMDigitizerModule::beginRun()
{
}

void BKLMDigitizerModule::event()
{
  //---------------------------------------------
  // Get BKLM hits collection from the data store
  //---------------------------------------------
  StoreArray<BKLMSimHit> simHits;
  unsigned int nSimHit = simHits.getEntries();
  if (nSimHit == 0) return;

  StoreArray<BKLMDigit> strips;
  RelationArray simHitToStrip(simHits, strips);

  unsigned int nStrip = 0;
  unsigned int s = 0;

  vector<unsigned int> indices;
  for (unsigned int h = 0; h < nSimHit; ++h) {
    BKLMSimHit* simHit = simHits[h];
    indices.clear();
    if (simHit->getStripMin() >= 0) {
      for (int j = simHit->getStripMin(); j <= simHit->getStripMax(); ++j) {
        BKLMDigit strip(simHit->getStatus(), simHit->isForward(), simHit->getSector(),
                        simHit->getLayer(), simHit->isPhiReadout(), j, simHit->getTime(), simHit->getDeltaE());
        indices.push_back(j);
        for (s = 0; s < nStrip; ++s) {
          if (strip.match(strips[s])) break;
        }
        if (s == nStrip) {
          strips.appendNew(strip);
          nStrip++;
        }
      }
    }
    simHitToStrip.add(h, indices);  // 1 hit to many strips
  }
}

void BKLMDigitizerModule::endRun()
{
}

void BKLMDigitizerModule::terminate()
{
}

