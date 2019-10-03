/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/dataobjects/EKLMHitMomentum.h>

using namespace Belle2;

EKLMHitMomentum::EKLMHitMomentum()
{
  m_e = -1;
  m_pX = -1;
  m_pY = -1;
  m_pZ = -1;
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

TLorentzVector EKLMHitMomentum::getMomentum() const
{
  return TLorentzVector(m_pX, m_pY, m_pZ, m_e);
}

