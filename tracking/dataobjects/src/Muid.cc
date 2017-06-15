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

Muid::Muid() :
  m_PDGCode(0),
  m_MuonPDFValue(0.0),
  m_PionPDFValue(0.0),
  m_KaonPDFValue(0.0),
  m_ProtonPDFValue(0.0),
  m_DeuteronPDFValue(0.0),
  m_ElectronPDFValue(0.0),
  m_JunkPDFValue(0.0),
  m_LogL_mu(-1.0E20),
  m_LogL_pi(-1.0E20),
  m_LogL_K(-1.0E20),
  m_LogL_p(-1.0E20),
  m_LogL_d(-1.0E20),
  m_LogL_e(-1.0E20),
  m_Status(0),
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
  m_PDGCode(pdgCode),
  m_MuonPDFValue(0.0),
  m_PionPDFValue(0.0),
  m_KaonPDFValue(0.0),
  m_ProtonPDFValue(0.0),
  m_DeuteronPDFValue(0.0),
  m_ElectronPDFValue(0.0),
  m_JunkPDFValue(0.0),
  m_LogL_mu(-1.0E20),
  m_LogL_pi(-1.0E20),
  m_LogL_K(-1.0E20),
  m_LogL_p(-1.0E20),
  m_LogL_d(-1.0E20),
  m_LogL_e(-1.0E20),
  m_Status(0),
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
