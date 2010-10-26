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
  m_Name = "";
  m_XStrip = NULL;
  m_YStrip = NULL;
}
EKLMHit2d::EKLMHit2d(const char * name)
{
  m_Name = name;
  m_XStrip = NULL;
  m_YStrip = NULL;
}


EKLMHit2d::EKLMHit2d(const std::string & name)
{
  m_Name = name;
  m_XStrip = NULL;
  m_YStrip = NULL;
}


EKLMHit2d::EKLMHit2d(EKLMStripHit * xStrip, EKLMStripHit * yStrip)
{
  m_Name = xStrip->getName() + "_x_" + yStrip->getName();
  m_XStrip = xStrip;
  m_YStrip = yStrip;
}

void EKLMHit2d::Print()
{
  std::cout << "------------  Hit 2d  -------------- " << std::endl;
  //  std::cout << m_Name << std::endl;
  std::cout << "X: ";
  m_XStrip->Print();
  std::cout << "Y: ";
  m_YStrip->Print();
  std::cout << "intersection: " << m_crossPoint << std::endl;
  std::cout << "ChiSq: " << m_ChiSq << std::endl;

}


bool EKLMHit2d::addStripHit(EKLMStripHit *stripHit)
{
  // important! getName() is case sensetive!!

  if (m_XStrip != NULL && m_YStrip != NULL) {
    FATAL("Attempt to add more than 2 strips in 2d hit!");
    exit(0);
  }
  if (m_XStrip == NULL && EKLMNameManipulator::isX(stripHit->getName())) {
    m_XStrip = stripHit;
    m_Name = stripHit->getName() + "_x_" + m_Name;
    return true;
  }
  if (m_YStrip == NULL && !EKLMNameManipulator::isX(stripHit->getName())) {
    m_YStrip = stripHit;
    m_Name = m_Name + "_x_" + stripHit->getName();
    return true;
  }
  return false;
}

void EKLMHit2d::setChiSq()
{

  double v = 17.0;  // 17cm/ns=speed of light; should be accessible via xml!
  double tX = m_XStrip->getTime() - m_XStrip->getLightPropagationLength(m_crossPoint) / v;
  double tY = m_YStrip->getTime() - m_YStrip->getLightPropagationLength(m_crossPoint) / v;

  double sigmaT = 1.;  // ns, smearing in time ; should be accessible via xml!
  m_ChiSq = (tX - tY) * (tX - tY) / sigmaT / sigmaT;
}



