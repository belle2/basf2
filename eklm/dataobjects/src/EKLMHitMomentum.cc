/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMHitMomentum.h>

using namespace Belle2;

ClassImp(EKLMHitMomentum);

EKLMHitMomentum::EKLMHitMomentum()
{
}

EKLMHitMomentum::~EKLMHitMomentum()
{
}

void EKLMHitMomentum::setMomentum(CLHEP::HepLorentzVector p)
{
  m_e = p.e();
  m_pX = p.px();
  m_pY = p.py();
  m_pZ = p.pz();
}

CLHEP::HepLorentzVector EKLMHitMomentum::getMomentum() const
{
  return CLHEP::HepLorentzVector(m_pX, m_pY, m_pZ, m_e);
}

TLorentzVector EKLMHitMomentum::getMomentumRoot() const
{
  return TLorentzVector(m_pX, m_pY, m_pZ, m_e);
}

