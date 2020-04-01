/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/eklm/EKLMHit2d.h>

using namespace Belle2;

EKLMHit2d::EKLMHit2d()
{
  m_ChiSq = -1;
}


EKLMHit2d::EKLMHit2d(EKLMDigit* s1)
{
  setSection(s1->getSection());
  setLayer(s1->getLayer());
  setSector(s1->getSector());
  m_ChiSq = -1;
}

EKLMHit2d::~EKLMHit2d()
{
}

float EKLMHit2d::getChiSq() const
{
  return m_ChiSq;
}

void EKLMHit2d::setChiSq(float chisq)
{
  m_ChiSq = chisq;
}

