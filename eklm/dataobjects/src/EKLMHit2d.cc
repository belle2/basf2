/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* System headers. */
#include <stdio.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMHit2d.h>

using namespace Belle2;

ClassImp(Belle2::EKLMHit2d);

EKLMHit2d::EKLMHit2d()
{
}


EKLMHit2d::EKLMHit2d(EKLMDigit* s1, EKLMDigit* s2)
{
  m_digit[0] = s1;
  m_digit[1] = s2;
  setEndcap(s1->getEndcap());
  setLayer(s1->getLayer());
  setSector(s1->getSector());
}

EKLMHit2d::~EKLMHit2d()
{
}

EKLMDigit* EKLMHit2d::getDigit(int plane) const
{
  return m_digit[plane - 1];
}

int EKLMHit2d::getStrip(int plane)
{
  return m_digit[plane - 1]->getStrip();
}

float EKLMHit2d::getChiSq() const
{
  return m_ChiSq;
}

void EKLMHit2d::setChiSq(float chisq)
{
  m_ChiSq = chisq;
}

