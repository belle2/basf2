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
#include <generators/dataobjects/MCParticle.h>
//#include <framework/gearbox/Unit.h>
//#include <framework/logging/Logger.h>

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

  //! Set description
  setDescription("BKLMDigitizerModule");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
}

BKLMDigitizerModule::~BKLMDigitizerModule()
{
}

void BKLMDigitizerModule::initialize()
{
  // Force creation of BKLM datastores
  StoreArray<BKLMSimHit> simHits;
  StoreArray<MCParticle> particles;
  RelationArray particleToSimHits(particles, simHits);
  StoreArray<BKLMDigit> strips;
  RelationArray stripToSimHits(strips, simHits);
  RelationArray simHitToStrip(simHits, strips);
  StoreArray<BKLMSimHit>::registerPersistent();
  StoreArray<BKLMDigit>::registerPersistent();
  RelationArray::registerPersistent<MCParticle, BKLMSimHit>();
  RelationArray::registerPersistent<BKLMDigit, BKLMSimHit>();
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
  if (!simHits) {
    B2ERROR("BKLMDigitizerModule: Cannot find BKLMSimHits array");
  }

  unsigned int nSimHit = simHits.getEntries();
  if (nSimHit == 0) return;

  StoreArray<BKLMDigit> strips("BKLMDigits");
  RelationArray stripToSimHits(strips, simHits);
  RelationArray simHitToStrip(simHits, strips);

  unsigned int nStrip = 0;
  unsigned int s = 0;

  vector<unsigned int> indices;
  vector<vector<unsigned int> > reverseIndices;
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
          new(strips.nextFreeAddress()) BKLMDigit(strip);
          vector<unsigned int> rev;
          reverseIndices.push_back(rev);
          nStrip++;
        }
        reverseIndices[s].push_back(h);
      }
    }
    simHitToStrip.add(h, indices);  // 1 hit to many strips
  }
  for (s = 0; s < nStrip; ++s) {
    stripToSimHits.add(s, reverseIndices[s]); // 1 strip to many hits
  }

}

void BKLMDigitizerModule::endRun()
{
}

void BKLMDigitizerModule::terminate()
{
}

void BKLMDigitizerModule::printModuleParameters() const
{
}
