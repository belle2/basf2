/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/receklm/EKLMRecon.h>

#include <framework/datastore/StoreIter.h>
#include <framework/datastore/DataStore.h>
#include <eklm/eklmutils/EKLMutils.h>



using namespace std;
namespace Belle2 {


  void EKLMRecon::readStripHits()
  {
    StoreArray<EKLMStripHit> stripHitsArray;
    for (int i = 0; i < stripHitsArray.getEntries(); i++)
      m_StripHitVector.push_back(stripHitsArray[i]);
  }


  void EKLMRecon::createSectorHits()
  {

    //    EKLMSectorHit *newSectorHit;
    StoreArray<EKLMSectorHit> sectorHitsArray;

    for (std::vector<EKLMStripHit*>::iterator stripIter =
           m_StripHitVector.begin(); stripIter != m_StripHitVector.end();
         ++stripIter) {
      bool sectorNotFound = true;
      for (std::vector<EKLMSectorHit*>::iterator sectorIter =
             m_SectorHitVector.begin(); sectorIter != m_SectorHitVector.end();
           sectorIter++) {
        // since every hit could be added only once
        if ((*sectorIter)->addStripHit(*stripIter)) {
          sectorNotFound = false;
          break;
        }
      }
      if (sectorNotFound) {
        EKLMSectorHit *newSectorHit = new(sectorHitsArray->AddrAt(sectorHitsArray.getEntries()))
        EKLMSectorHit((*stripIter)->getEndcap(),
                      (*stripIter)->getLayer(),
                      (*stripIter)->getSector(), 0, 0);

        newSectorHit->addStripHit(*stripIter);
        m_SectorHitVector.push_back(newSectorHit);
      }
    }
  }

  void EKLMRecon::create2dHits()
  {
    // loop over sectors
    for (std::vector<EKLMSectorHit*>::iterator sectorIter =
           m_SectorHitVector.begin(); sectorIter != m_SectorHitVector.end();
         sectorIter++)
      (*sectorIter)->create2dHits();
  }



}//namespace
