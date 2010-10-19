/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/eklmhit/EKLMHit2d.h>
#include <eklm/eklmutils/EKLMutils.h>
#include <framework/logging/Logger.h>


using namespace Belle2;
using namespace std;

ClassImp(EKLMHit2d);

EKLMHit2d::EKLMHit2d()
{
  m_XStrip = NULL;
  m_YStrip = NULL;
}
EKLMHit2d::EKLMHit2d(const char * name)
{
  m_Name = name;
  m_XStrip = NULL;
  m_YStrip = NULL;
}

EKLMHit2d::EKLMHit2d(std::string & name)
{
  m_Name = name;
  m_XStrip = NULL;
  m_YStrip = NULL;
}


void EKLMHit2d::Print()
{
  std::cout << "------------  Hit 2d  -------------- " << std::endl;
  std::cout << m_Name << std::endl;
  std::cout << "X: ";
  m_XStrip->Print();
  std::cout << "Y: ";
  m_YStrip->Print();
}


bool EKLMHit2d::addStripHit(EKLMStripHit *stripHit)
{
  // important! getName() is case sensetive!!

  if (m_XStrip != NULL && m_YStrip != NULL) {
    FATAL("Attempt to add more than 2 strips in 2d hit!");
    exit(0);
  }


  return false;
}




