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
#include <eklm/eklmutils/EKLMutils.h>
#include <framework/logging/Logger.h>


using namespace Belle2;
using namespace std;

ClassImp(Belle2::EKLMHit2d);

EKLMHit2d::EKLMHit2d()
{
  m_XStrip = NULL;
  m_YStrip = NULL;
}

const Belle2::EKLMStripHit* EKLMHit2d::getXStripHit() const
{
  return m_XStrip;
}

const Belle2::EKLMStripHit* EKLMHit2d::getYStripHit() const
{
  return m_YStrip;
}

void EKLMHit2d::setCrossPoint(TVector3 & point)
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

EKLMHit2d::EKLMHit2d(EKLMStripHit * xStrip, EKLMStripHit * yStrip)
{
  m_XStrip = xStrip;
  m_YStrip = yStrip;
  setEndcap(xStrip->getEndcap());
  setLayer(xStrip->getLayer());
  setSector(xStrip->getSector());
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


bool EKLMHit2d::addStripHit(const EKLMStripHit *stripHit)
{
  if (m_XStrip != NULL && m_YStrip != NULL) {
    B2FATAL("Attempt to add more than 2 strips in 2d hit!");
    exit(0);
  }
  if (m_XStrip == NULL && CheckStripOrientationX(stripHit->getVolume())) {
    m_XStrip = stripHit;
    return true;
  }
  if (m_YStrip == NULL && !CheckStripOrientationX(stripHit->getVolume())) {
    m_YStrip = stripHit;
    return true;
  }
  return false;
}

void EKLMHit2d::setChiSq()
{

  double v = 17.0;  // 17cm/ns=speed of light; should be accessible via xml!
  double tX = m_XStrip->getTime() -
              getLightPropagationLength(m_XStrip->getVolume(), m_crossPoint) / v;
  double tY = m_YStrip->getTime() -
              getLightPropagationLength(m_YStrip->getVolume(), m_crossPoint) / v;

  double sigmaT = 1.;  // ns, smearing in time ; should be accessible via xml!
  m_ChiSq = (tX - tY) * (tX - tY) / sigmaT / sigmaT;
}

