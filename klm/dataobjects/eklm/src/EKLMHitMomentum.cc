/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/eklm/EKLMHitMomentum.h>

using namespace Belle2;

EKLMHitMomentum::EKLMHitMomentum() :
  m_e(-1),
  m_pX(-1),
  m_pY(-1),
  m_pZ(-1)
{
}

EKLMHitMomentum::~EKLMHitMomentum()
{
}

void EKLMHitMomentum::setMomentum(const TLorentzVector& p)
{
  m_e = p.E();
  m_pX = p.Px();
  m_pY = p.Py();
  m_pZ = p.Pz();
}
