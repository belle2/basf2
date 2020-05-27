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

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMHit2d::EKLMHit2d() :
  m_ChiSq(-1)
{
}


EKLMHit2d::EKLMHit2d(KLMDigit* s1) :
  m_ChiSq(-1)
{
  if (s1->getSubdetector() != KLMElementNumbers::c_EKLM)
    B2FATAL("Trying to construct a EKLMHit2d using KLMDigits from BKLM.");
  setSection(s1->getSection());
  setLayer(s1->getLayer());
  setSector(s1->getSector());
}

EKLMHit2d::~EKLMHit2d()
{
}
