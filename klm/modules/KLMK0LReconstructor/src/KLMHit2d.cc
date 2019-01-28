/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <klm/modules/KLMK0LReconstructor/KLMHit2d.h>

using namespace Belle2;

KLMHit2d::KLMHit2d(BKLMHit2d* bklmHit2d)
{
  m_bklmHit = true;
  m_bklmHit2d = bklmHit2d;
  m_eklmHit2d = nullptr;
}

KLMHit2d::KLMHit2d(EKLMHit2d* eklmHit2d)
{
  m_bklmHit = false;
  m_bklmHit2d = nullptr;
  m_eklmHit2d = eklmHit2d;
}

KLMHit2d::~KLMHit2d()
{
}

bool KLMHit2d::inBKLM() const
{
  return m_bklmHit;
}

BKLMHit2d* KLMHit2d::getBKLMHit2d() const
{
  return m_bklmHit2d;
}

EKLMHit2d* KLMHit2d::getEKLMHit2d() const
{
  return m_eklmHit2d;
}

TVector3 KLMHit2d::getPosition() const
{
  if (m_bklmHit)
    return m_bklmHit2d->getGlobalPosition();
  return m_eklmHit2d->getPosition();
}

float KLMHit2d::getTime() const
{
  if (m_bklmHit)
    return m_bklmHit2d->getTime();
  return m_eklmHit2d->getTime();
}

int KLMHit2d::getLayer() const
{
  if (m_bklmHit)
    return m_bklmHit2d->getLayer();
  return m_eklmHit2d->getLayer();
}

