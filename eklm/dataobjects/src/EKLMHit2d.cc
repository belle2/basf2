/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/dataobjects/EKLMHit2d.h>



using namespace Belle2;
using namespace std;

ClassImp(Belle2::EKLMHit2d);

EKLMHit2d::EKLMHit2d()
{
  m_XStrip = NULL;
  m_YStrip = NULL;
}


EKLMHit2d::EKLMHit2d(EKLMDigit* xStrip, EKLMDigit* yStrip)
{

  m_XStrip = xStrip;
  m_YStrip = yStrip;
  setEndcap(xStrip->getEndcap());
  setLayer(xStrip->getLayer());
  setSector(xStrip->getSector());
}


const Belle2::EKLMDigit* EKLMHit2d::getXStripHit() const
{
  return m_XStrip;
}

const Belle2::EKLMDigit* EKLMHit2d::getYStripHit() const
{
  return m_YStrip;
}

void EKLMHit2d::setCrossPoint(TVector3& point)
{
  m_crossPoint = point;
}

TVector3 EKLMHit2d::getCrossPoint() const
{
  return m_crossPoint;
}

double EKLMHit2d::getChiSq() const
{
  return m_ChiSq;
}

void EKLMHit2d::Print()
{
  std::cout << "------------  Hit 2d  -------------- " << std::endl;
  std::cout << "Endcap: " << getEndcap()
            << " Layer: " << getLayer()
            << " Sector: " << getSector() << "\n";
  std::cout << "X: ";
  m_XStrip->Print();
  std::cout << "Y: ";
  m_YStrip->Print();
  std::cout << "intersection:";
  m_crossPoint.Print();

  std::cout << "ChiSq: " << m_ChiSq << std::endl;
}

void EKLMHit2d::setChiSq(double chisq)
{
  m_ChiSq = chisq;
}
