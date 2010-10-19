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
  for (std::vector<EKLMStripHit*>::iterator it = m_stripHitVector.begin(); it != m_stripHitVector.end(); ++it)
    (*it)->Print();
}


bool EKLMSectorHit::addStripHit(EKLMStripHit *stripHit)
{
  // important! getName is case sensetive!!
  if (EKLMNameManipulator::getSectorName(stripHit->getName()) == m_Name) {
    m_stripHitVector.push_back(stripHit);
    return true;
  }
  return false;
}




