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

  void EKLMDigitizer::readSimHits()
  {
    StoreArray<EKLMSimHit> array("SimHitsEKLMArray");
    for (int i = 0; i < array.GetEntries(); i++)
      m_simHitsVector.push_back(array[i]);
  }

  void EKLMDigitizer::sortSimHits()
  {
    for (std::vector<EKLMSimHit*>::iterator iHit = m_simHitsVector.begin();
         iHit != m_simHitsVector.end(); ++iHit) {
      std::string StripName = (*iHit)->getPV()->GetName();

      // search for entries of the same strip
      std::map<std::string, std::vector<EKLMSimHit*> >::iterator
      it = m_HitStripMap.find(StripName);

      if (it == m_HitStripMap.end()) { //  new entry
        std::vector<EKLMSimHit*> *vectorHits =
          new std::vector<EKLMSimHit*> (1, (*iHit));
        m_HitStripMap.insert(std::pair<std::string, std::vector<EKLMSimHit*> >
                             (StripName, *vectorHits));
      } else {
        it->second.push_back(*iHit);
      }
    }

  }

  //!  This function is  to form StripHits from SimHits.
  //!  Light propagation into the fiber, SiPM and electronics effects
  //!  are simulated in EKLMFiberAndElectronics class
  void EKLMDigitizer::mergeSimHitsToStripHits()
  {
    //    B2INFO( "STRAT MERGING HITS");
    for (std::map<std::string, std::vector<EKLMSimHit*> >::iterator it =
           m_HitStripMap.begin(); it != m_HitStripMap.end(); it++) {

      //      B2DEBUG(1, "MAP ENTRY");
      EKLMFiberAndElectronics * fiberAndElectronicsSimulator =
        new EKLMFiberAndElectronics(*it);
      fiberAndElectronicsSimulator->processEntry();

      EKLMSimHit *simHit = NULL;
      EKLMStripHit *stripHit = new EKLMStripHit();

      stripHit->setName(it->first);
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

      m_HitVector.push_back(stripHit);
      delete fiberAndElectronicsSimulator;
    }
    //    B2INFO( "STOP MERGING HITS");
  }

  void EKLMDigitizer::saveStripHits()
  {

    for (std::vector<EKLMStripHit*>::const_iterator iter = m_HitVector.begin();
         iter != m_HitVector.end(); ++iter)
      storeEKLMObject("StripHitsEKLMArray", *iter);

  }
}

