/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <eklm/simeklm/EKLMFiberAndElectronics.h>
#include <eklm/simeklm/EKLMDigitizer.h>
#include <eklm/eklmutils/EKLMutils.h>
#include <framework/logging/Logger.h>
#include<iostream>
#include<fstream>

//#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>

#include <TRandom.h>

#include <geometry/CreatorManager.h>
#include <geometry/CreatorBase.h>

#include <eklm/geoeklm/GeoEKLMBelleII.h>


using namespace CLHEP;

namespace Belle2 {

  G4Allocator<EKLMDigitizer> EKLMDigitizerAllocator;

  void* EKLMDigitizer::operator new(size_t)
  {
    void *aEKLMDigitizer;
    aEKLMDigitizer = (void *) EKLMDigitizerAllocator.MallocSingle();
    return aEKLMDigitizer;
  }

  void EKLMDigitizer::operator delete(void *aEKLMDigitizer)
  {
    EKLMDigitizerAllocator.FreeSingle((EKLMDigitizer*) aEKLMDigitizer);
  }
  /*
  void EKLMDigitizer::readAndSortBkgHits()
  {
    B2DEBUG(1,"EKLMDigitizer::readAndSortBkgHits()");

    StoreArray<EKLMBackHit> bkgHitsArray;
    for (int i = 0; i < bkgHitsArray.getEntries(); i++) {

      // search for entries of the same strip
      map<string, vector<EKLMBackHit*> >::iterator
  it = m_HitVolumeMap.find((bkgHitsArray[i])->getName());

      if (it == m_HitVolumeMap.end()) { //  new entry
      vector<EKLMBackHit*> *vectorHits =
        new vector<EKLMBackHit*> (1, (bkgHitsArray[i]));
      m_HitVolumeMap.insert(pair<string, vector<EKLMBackHit*> >
                            ((bkgHitsArray[i])->getName(), *vectorHits));
      } else {
  it->second.push_back(bkgHitsArray[i]);
      }
    }

    B2DEBUG(1,"EKLMDigitizer::readAndSortBkgHits()  completed");
  }
  */









  void EKLMDigitizer::readAndSortSimHits()
  {
    StoreArray<EKLMSimHit> simHitsArray;
    for (int i = 0; i < simHitsArray.getEntries(); i++) {

      // search for entries of the same strip
      std::map<G4VPhysicalVolume *, std::vector<EKLMSimHit*> >::iterator
      it = m_HitStripMap.find((simHitsArray[i])->getPV());

      if (it == m_HitStripMap.end()) { //  new entry
        std::vector<EKLMSimHit*> *vectorHits =
          new std::vector<EKLMSimHit*> (1, (simHitsArray[i]));
        m_HitStripMap.insert(std::pair<G4VPhysicalVolume *, std::vector<EKLMSimHit*> >
                             ((simHitsArray[i])->getPV(), *vectorHits));
      } else {
        it->second.push_back(simHitsArray[i]);
      }
    }

  }

  //!  This function is  to form StripHits from SimHits.
  //!  Light propagation into the fiber, SiPM and electronics effects
  //!  are simulated in EKLMFiberAndElectronics class
  void EKLMDigitizer::mergeSimHitsToStripHits()
  {
    //    B2INFO( "STRAT MERGING HITS");
    for (std::map<G4VPhysicalVolume *, std::vector<EKLMSimHit*> >::iterator it =
           m_HitStripMap.begin(); it != m_HitStripMap.end(); it++) {

      //      B2DEBUG(1, "MAP ENTRY");
      EKLMFiberAndElectronics * fiberAndElectronicsSimulator =
        new EKLMFiberAndElectronics(*it);
      fiberAndElectronicsSimulator->processEntry();

      EKLMSimHit *simHit = it->second.front();

      StoreArray<EKLMStripHit> stripHitsArray;
      EKLMStripHit *stripHit = new(stripHitsArray->AddrAt(stripHitsArray.getEntries()))EKLMStripHit();

      //      EKLMStripHit *stripHit = new EKLMStripHit();

      stripHit->set_nEndcap(simHit->get_nEndcap());
      stripHit->set_nLayer(simHit->get_nLayer());
      stripHit->set_nSector(simHit->get_nSector());
      stripHit->set_nPlane(simHit->get_nPlane());
      stripHit->set_nStrip(simHit->get_nStrip());
      stripHit->setPV(simHit->getPV());

      //stripHit->setHistogramm(cloneHist);

      if (!fiberAndElectronicsSimulator->getFitStatus()) {
        stripHit->setTime(fiberAndElectronicsSimulator->getFitResults(0));
        stripHit->setNumberPhotoElectrons(fiberAndElectronicsSimulator->
                                          getFitResults(3));
      } else {
        stripHit->setNumberPhotoElectrons(-1);
        stripHit->setTime(-1);
      }

      stripHit->setLeadingParticlePDGCode(0);

      //      m_HitVector.push_back(stripHit);
      delete fiberAndElectronicsSimulator;
    }
    //    B2INFO( "STOP MERGING HITS");
  }
}

