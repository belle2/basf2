/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
