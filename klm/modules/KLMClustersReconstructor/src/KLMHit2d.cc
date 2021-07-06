/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMClustersReconstructor/KLMHit2d.h>

using namespace Belle2;

KLMHit2d::KLMHit2d(BKLMHit2d* bklmHit2d) :
  m_bklmHit(true),
  m_bklmHit2d(bklmHit2d),
  m_eklmHit2d(nullptr)
{
}

KLMHit2d::KLMHit2d(EKLMHit2d* eklmHit2d) :
  m_bklmHit(false),
  m_bklmHit2d(nullptr),
  m_eklmHit2d(eklmHit2d)
{
}

KLMHit2d::~KLMHit2d()
{
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

