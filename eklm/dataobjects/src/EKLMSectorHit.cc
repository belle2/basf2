/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TVector3.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMSectorHit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(Belle2::EKLMSectorHit)

EKLMSectorHit::EKLMSectorHit(int nEndcap, int nLayer, int  nSector) :
  EKLMHitBase(nEndcap, nLayer, nSector)
{
}

std::vector <EKLMDigit*> * EKLMSectorHit::getStripHitVector()
{
  return & m_stripHitVector;
}

std::vector <EKLMHit2d*> * EKLMSectorHit::get2dHitVector()
{
  return  & m_hit2dVector;
}

void EKLMSectorHit::Print()
{
  std::cout << "------------  Sector Hit  -------------- " << std::endl;
  std::cout << "Endcap: " << getEndcap()
            << " Layer: " << getLayer()
            << " Sector: " << getSector() << std::endl;
  for (std::vector<EKLMDigit*>::iterator it = m_stripHitVector.begin();
       it != m_stripHitVector.end(); ++it)
    (*it)->Print();
}

