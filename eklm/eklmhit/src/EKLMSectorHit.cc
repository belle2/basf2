/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/eklmhit/EKLMSectorHit.h>
#include <eklm/eklmutils/EKLMutils.h>
#include <framework/logging/Logger.h>


using namespace Belle2;
using namespace std;

ClassImp(EKLMSectorHit);

EKLMSectorHit::EKLMSectorHit(const char * name)
{
  m_Name = name;
}

EKLMSectorHit::EKLMSectorHit(std::string & name)
{
  m_Name = name;
}


void EKLMSectorHit::Print()
{
  std::cout << "------------  Sector Hit  -------------- " << std::endl;
  std::cout << m_Name << std::endl;
  for (std::vector<EKLMStripHit*>::iterator it = m_stripHitVector.begin();
       it != m_stripHitVector.end(); ++it)
    (*it)->Print();
}


bool EKLMSectorHit::addStripHit(EKLMStripHit *stripHit)
{
  // important! getName is case sensetive!!
  if (EKLMNameManipulator::getVolumeName(stripHit->getName(), "Sector") ==
      m_Name) {
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
    if (!EKLMNameManipulator::isX((*itX)->getName()))
      continue;
    for (std::vector<EKLMStripHit*>::iterator itY = m_stripHitVector.begin();
         itY != m_stripHitVector.end(); ++itY) {
      // only Y strips
      if (EKLMNameManipulator::isX((*itY)->getName()))
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

