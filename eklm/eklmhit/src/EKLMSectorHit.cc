/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/eklmhit/EKLMSectorHit.h>
#include <eklm/eklmutils/EKLMutils.h>
#include <framework/logging/Logger.h>


using namespace Belle2;
using namespace std;

ClassImp(EKLMSectorHit);

EKLMSectorHit::EKLMSectorHit(char nEndcap, char nLayer, char nSector,
                             char nPlane, char nStrip) :
    EKLMHitBase(nEndcap, nLayer, nSector, nPlane, nStrip)
{
}

void EKLMSectorHit::Print()
{
  std::cout << "------------  Sector Hit  -------------- " << std::endl;
  std::cout << "Endcap: " << get_nEndcap()
            << " Layer: " << get_nLayer()
            << " Sector: " << get_nSector() << "\n";
  for (std::vector<EKLMStripHit*>::iterator it = m_stripHitVector.begin();
       it != m_stripHitVector.end(); ++it)
    (*it)->Print();
}

bool EKLMSectorHit::addStripHit(EKLMStripHit *stripHit)
{
  if (stripHit->get_nEndcap() == get_nEndcap() &&
      stripHit->get_nLayer() == get_nLayer() &&
      stripHit->get_nSector() == get_nSector()) {
    m_stripHitVector.push_back(stripHit);
    return true;
  }
  return false;
}


void EKLMSectorHit::create2dHits()
{

  for (std::vector<EKLMStripHit*>::iterator itX = m_stripHitVector.begin();
       itX != m_stripHitVector.end(); ++itX) {
    // only X strips
    if (!CheckStripOrientationX((*itX)->getPV()))
      continue;
    for (std::vector<EKLMStripHit*>::iterator itY = m_stripHitVector.begin();
         itY != m_stripHitVector.end(); ++itY) {
      // only Y strips
      if (CheckStripOrientationX((*itY)->getPV()))
        continue;
      CLHEP::Hep3Vector crossPoint(0, 0, 0);
      // drop entries with non-intersected strips
      if (!((*itX)->doesIntersect(*itY, crossPoint)))
        continue;
      EKLMHit2d *hit2d = new EKLMHit2d(*itX, *itY);
      hit2d->setCrossPoint(crossPoint);
      hit2d->setChiSq();
      m_hit2dVector.push_back(hit2d);
      //      hit2d->Print();
    }

  }
}


void EKLMSectorHit::store2dHits()
{
  for (std::vector<EKLMHit2d*>::iterator it = m_hit2dVector.begin();
       it != m_hit2dVector.end(); ++it)
    storeEKLMObject("Hits2dEKLMArray", *it);
}

