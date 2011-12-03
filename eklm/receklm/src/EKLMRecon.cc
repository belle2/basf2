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
    /*
    //    EKLMSectorHit *newSectorHit;
    StoreArray<EKLMSectorHit> sectorHitsArray;

    for (vector<EKLMStripHit*>::iterator stripIter =
           m_StripHitVector.begin(); stripIter != m_StripHitVector.end();
         ++stripIter) {
      bool sectorNotFound = true;
      for (vector<EKLMSectorHit*>::iterator sectorIter =
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
    */
  }

  void EKLMRecon::create2dHits()
  {
    // loop over sectors
    for (vector<EKLMSectorHit*>::iterator sectorIter =
           m_SectorHitVector.begin(); sectorIter != m_SectorHitVector.end();
         sectorIter++) {

      vector<EKLMStripHit*>::iterator itX = ((*sectorIter)->getStripHitVector())->begin();

      for (vector<EKLMStripHit*>::iterator itX = (*sectorIter)->getStripHitVector()->begin();
           itX != (*sectorIter)->getStripHitVector()->end(); ++itX) {
        // only X strips
        if (!CheckStripOrientationX((*itX)->getVolume()))
          continue;
        for (vector<EKLMStripHit*>::iterator itY = (*sectorIter)->getStripHitVector()->begin();
             itY != (*sectorIter)->getStripHitVector()->end(); ++itY) {
          // only Y strips
          if (CheckStripOrientationX((*itY)->getVolume()))
            continue;
          TVector3 crossPoint(0, 0, 0);
          // drop entries with non-intersected strips
          if (!(doesIntersect(*itX, *itY, crossPoint)))
            continue;

          EKLMHit2d *hit2d = new(m_hit2dArray->AddrAt(m_hit2dArray.getEntries()))EKLMHit2d(*itX, *itY);
          hit2d->setCrossPoint(crossPoint);
          hit2d->setChiSq();
          m_hit2dVector.push_back(hit2d);
          //hit2d->Print();
        }
      }
    }
  }


}//namespace
