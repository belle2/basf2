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
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <generators/dataobjects/MCParticle.h>

#include <bklm/dataobjects/BKLMSimHit.h>
#include <bklm/dataobjects/BKLMStrip.h>

#include <TVector3.h>

#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <utility> //contains pair

using namespace std;
using namespace boost;
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
  StoreArray<BKLMStrip> strips;
  RelationArray stripToSimHits(strips, simHits);
  RelationArray simHitToStrip(simHits, strips);
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

  unsigned int nSimHit = simHits->GetEntriesFast();
  if (nSimHit == 0) return;

  StoreArray<BKLMStrip> strips("BKLMStrips");
  RelationArray stripToSimHits(strips, simHits);
  RelationArray simHitToStrip(simHits, strips);

  unsigned int nStrip = 0;
  unsigned int s = 0;
  BKLMStrip* strip = new BKLMStrip();

  vector<unsigned int> indices;
  vector<vector<unsigned int> > reverseIndices;
  for (unsigned int h = 0; h < nSimHit; ++h) {
    BKLMSimHit* simHit = simHits[h];
    indices.clear();
    strip->setInRPC(simHit->getInRPC());
    strip->setFrontBack(simHit->getFrontBack());
    strip->setSector(simHit->getSector());
    strip->setLayer(simHit->getLayer());
    strip->setTDC(simHit->getHitTime());
    strip->setADC(simHit->getDeltaE());
    strip->setDirection('P');
    for (std::vector<int>::const_iterator iS = simHit->getPhiStrips()->begin();
         iS != simHit->getPhiStrips()->end(); ++iS) {
      indices.push_back(*iS);
      strip->setStrip(*iS);
      for (s = 0; s < nStrip; ++s) {
        if (strip->match(strips[s])) break;
      }
      if (s == nStrip) {
        new(strips->AddrAt(s)) BKLMStrip(*strip);
        vector<unsigned int> rev;
        reverseIndices.push_back(rev);
        nStrip++;
      }
      reverseIndices[s].push_back(h);
    }
    strip->setDirection('Z');
    for (std::vector<int>::const_iterator iS = simHit->getZStrips()->begin();
         iS != simHit->getZStrips()->end(); ++iS) {
      indices.push_back(*iS);
      strip->setStrip(*iS);
      for (s = 0; s < nStrip; ++s) {
        if (strip->match(strips[s])) break;
      }
      if (s == nStrip) {
        new(strips->AddrAt(s)) BKLMStrip(*strip);
        vector<unsigned int> rev;
        reverseIndices.push_back(rev);
        nStrip++;
      }
      reverseIndices[s].push_back(h);
    }
    simHitToStrip.add(h, indices);  // 1 hit to many strips
    //new(simHitToStrip->AddrAt(h)) Relation(simHits, strips, h, indices);       // 1 hit to many strips
  }
  for (s = 0; s < nStrip; ++s) {
    stripToSimHits.add(s, reverseIndices[s]); // 1 strip to many hits
    //new(stripToSimHits->AddrAt(s)) Relation(strips, simHits, s, reverseIndices[s]);        // 1 strip to many hits
  }
  delete strip;

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
