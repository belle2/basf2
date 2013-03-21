/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/Muid.h>

using namespace std;
using namespace Belle2;

ClassImp(Muid)

Muid::Muid() :
  m_pdgCode(0),
  m_MuonPDFValue(0.0),
  m_PionPDFValue(0.0),
  m_KaonPDFValue(0.0),
  m_MissPDFValue(1.0),
  m_JunkPDFValue(0.0),
  m_ChiSquared(0.0),
  m_DegreesOfFreedom(0),
  m_Outcome(0),
  m_BarrelExtLayer(-1),
  m_EndcapExtLayer(-1),
  m_BarrelHitLayer(-1),
  m_EndcapHitLayer(-1),
  m_ExtLayer(-1),
  m_HitLayer(-1),
  m_ExtLayerPattern(0),
  m_HitLayerPattern(0)
{
}

Muid::Muid(int pdgCode) :
  m_pdgCode(pdgCode),
  m_MuonPDFValue(0.0),
  m_PionPDFValue(0.0),
  m_KaonPDFValue(0.0),
  m_MissPDFValue(1.0),
  m_JunkPDFValue(0.0),
  m_ChiSquared(0.0),
  m_DegreesOfFreedom(0),
  m_Outcome(0),
  m_BarrelExtLayer(-1),
  m_EndcapExtLayer(-1),
  m_BarrelHitLayer(-1),
  m_EndcapHitLayer(-1),
  m_ExtLayer(-1),
  m_HitLayer(-1),
  m_ExtLayerPattern(0),
  m_HitLayerPattern(0)
{
}
