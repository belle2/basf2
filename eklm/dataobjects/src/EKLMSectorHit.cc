/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/dataobjects/EKLMSectorHit.h>
#include <eklm/eklmutils/EKLMutils.h>
#include <framework/logging/Logger.h>
#include "TVector3.h"

using namespace Belle2;
using namespace std;

ClassImp(Belle2::EKLMSectorHit)

EKLMSectorHit::EKLMSectorHit(int nEndcap, int nLayer, int  nSector) :
    EKLMHitBase()
{}

vector <EKLMStripHit*> * EKLMSectorHit::getStripHitVector()
{
  return & m_stripHitVector;
}

vector <EKLMHit2d*> * EKLMSectorHit::get2dHitVector()
{
  return  & m_hit2dVector;
}

void EKLMSectorHit::Print()
{
  cout << "------------  Sector Hit  -------------- " << endl;
  cout << "Endcap: " << getEndcap()
       << " Layer: " << getLayer()
       << " Sector: " << getSector() << endl;
  for (vector<EKLMStripHit*>::iterator it = m_stripHitVector.begin();
       it != m_stripHitVector.end(); ++it)
    (*it)->Print();
}

bool EKLMSectorHit::addStripHit(EKLMStripHit *stripHit)
{
  if (stripHit->getEndcap() == getEndcap() &&
      stripHit->getLayer() == getLayer() &&
      stripHit->getSector() == getSector()) {
    m_stripHitVector.push_back(stripHit);
    return true;
  }
  return false;
}


