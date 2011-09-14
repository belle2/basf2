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
// DataStore to ask it for the iterator
#include <framework/datastore/DataStore.h>
// to have the EDurability type available
//#include <framework/datastore/StoreDefs.h>
#include <eklm/eklmutils/EKLMutils.h>



using namespace std;
namespace Belle2 {


  void EKLMRecon::readStripHits()
  {
    StoreArray<EKLMStripHit> array("StripHitsEKLMArray");
    for (int i = 0; i < array.getEntries(); i++)
      m_StripHitVector.push_back(array[i]);
  }


  void EKLMRecon::createSectorHits()
  {
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
        EKLMSectorHit  *newSectorHit = new EKLMSectorHit(EKLMNameManipulator::
                                                         getVolumeName((*stripIter)->getName(), "Sector").c_str());
        newSectorHit->addStripHit(*stripIter);
        m_SectorHitVector.push_back(newSectorHit);
      }
    }

    for (std::vector<EKLMSectorHit*>::iterator sectorIter =
           m_SectorHitVector.begin();
         sectorIter != m_SectorHitVector.end(); sectorIter++) {
      storeEKLMObject("SectorHitsEKLMArray", *sectorIter);
      //  (*sectorIter)->Print();
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


  void EKLMRecon::store2dHits()
  {
    // loop over sectors
    for (std::vector<EKLMSectorHit*>::iterator sectorIter =
           m_SectorHitVector.begin(); sectorIter != m_SectorHitVector.end();
         sectorIter++)
      (*sectorIter)->store2dHits();
  }


}//namespace
