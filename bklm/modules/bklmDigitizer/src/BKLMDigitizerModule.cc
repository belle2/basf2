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

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <bklm/dataobjects/BKLMSimHit.h>
#include <bklm/dataobjects/BKLMDigit.h>
#include <klm/simulation/ScintillatorSimulator.h>

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
  simHits.isRequired();

  // Force creation and persistence of BKLM output datastores
  bklmDigits.registerInDataStore();
  simHits.registerRelationTo(bklmDigits);
  m_Fitter = new KLM::ScintillatorFirmware(m_digitParams->getNDigitizations());
}

void BKLMDigitizerModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  B2DEBUG(1, "BKLMDigitizer: Experiment " << evtMetaData->getExperiment() << "  run " << evtMetaData->getRun());
}

void BKLMDigitizerModule::event()
{
  //---------------------------------------------
  // Get BKLM hits collection from the data store
  //---------------------------------------------
  int nSimHit = simHits.getEntries();
  if (nSimHit == 0) return;


  unsigned int nDigit = 0;
  unsigned int d = 0;

  std::multimap<int, BKLMSimHit*> volIDToSimHits;
  for (int h = 0; h < nSimHit; ++h) {
    BKLMSimHit* simHit = simHits[h];
    if (simHit->inRPC()) {
      if (simHit->getStripMin() > 0) {
        for (int s = simHit->getStripMin(); s <= simHit->getStripMax(); ++s) {
          int moduleID = (simHit->getModuleID() & ~BKLM_STRIP_MASK) | ((s - 1) << BKLM_STRIP_BIT);
          for (d = 0; d < nDigit; ++d) {
            if (((bklmDigits[d]->getModuleID() ^ moduleID) & BKLM_MODULESTRIPID_MASK) == 0) break;
          }
          if (d == nDigit) {
            bklmDigits.appendNew(simHit, s);
            nDigit++;
          } else {
            // DIVOT need pileup of RPC hits here
          }
          simHit->addRelationTo(bklmDigits[d]);  // 1 RPC hit to many digits
        }
      }
    } else {
      int volID = simHit->getModuleID() & BKLM_MODULESTRIPID_MASK;
      volIDToSimHits.insert(std::pair<int, BKLMSimHit*>(volID, simHit));
    }
  }

  // Digitize the scintillator-strip hits
  digitize(volIDToSimHits, bklmDigits);

}

void BKLMDigitizerModule::endRun()
{
}

void BKLMDigitizerModule::terminate()
{
  delete m_Fitter;
}

void BKLMDigitizerModule::digitize(std::multimap<int, BKLMSimHit*>& volIDToSimHits,
                                   StoreArray<BKLMDigit>& digits)
{
  int tdc;
  KLM::ScintillatorSimulator simulator(&(*m_digitParams), m_Fitter, 0, false);
  std::multimap<int, BKLMSimHit*>::iterator it, ub;
  for (it = volIDToSimHits.begin(); it != volIDToSimHits.end();
       it = volIDToSimHits.upper_bound(it->first)) {
    BKLMSimHit* simHit = it->second;
    ub = volIDToSimHits.upper_bound(it->first);
    simulator.simulate(it, ub);
    if (simulator.getGeneratedNPE() == 0)
      continue;
    BKLMDigit* bklmDigit = digits.appendNew(simHit);
    it->second->addRelationTo(bklmDigit);
    // Not implemented in BKLMDigit.
    // eklmDigit->setMCTime(simHit->getTime());
    // eklmDigit->setSiPMMCTime(simulator.getMCTime());
    // eklmDigit->setPosition(simHit->getPosition());
    bklmDigit->setSimNPixel(simulator.getGeneratedNPE());
    if (simulator.getFitStatus() == KLM::c_ScintillatorFirmwareSuccessfulFit) {
      tdc = simulator.getFPGAFit()->getStartTime();
      /* Differs from original BKLM definition! */
      bklmDigit->setCharge(simulator.getFPGAFit()->getMinimalAmplitude());
      bklmDigit->isAboveThreshold(true);
    } else {
      tdc = 0;
      bklmDigit->setCharge(0);
      bklmDigit->isAboveThreshold(false);
    }
    // Not implemented in BKLMDigit.
    // eklmDigit->setTDC(tdc);
    bklmDigit->setTime(m_TimeConversion->getTimeSimulation(tdc, true));
    bklmDigit->setFitStatus(simulator.getFitStatus());
    bklmDigit->setNPixel(simulator.getNPE());
    // Not implemented in simulator (original BKLM definition is wrong!). */
    // bklmDigit->setEDep(bklmDigit->getNPixel() / m_nPEperMeV);
    bklmDigit->setEDep(0);
  }
}
