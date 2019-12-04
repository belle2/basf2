/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/KLMMuidLikelihood.h>

using namespace std;
using namespace Belle2;

KLMMuidLikelihood::KLMMuidLikelihood() :
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
  for (int i = 0; i < BKLMElementNumbers::getMaximalLayerNumber(); ++i) {
    m_ExtBKLMEfficiencyValue[i] = 1.0;
  }
  for (int i = 0; i < EKLMElementNumbers::getMaximalLayerNumber(); ++i) {
    m_ExtEKLMEfficiencyValue[i] = 1.0;
  }
}

KLMMuidLikelihood::KLMMuidLikelihood(int pdgCode) :
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
  for (int i = 0; i < BKLMElementNumbers::getMaximalLayerNumber(); ++i) {
    m_ExtBKLMEfficiencyValue[i] = 1.0;
  }
  for (int i = 0; i < EKLMElementNumbers::getMaximalLayerNumber(); ++i) {
    m_ExtEKLMEfficiencyValue[i] = 1.0;
  }
}

unsigned int KLMMuidLikelihood::getTotalBarrelHits() const
{
  unsigned int hits = 0;
  unsigned int pattern = m_HitLayerPattern;
  for (int bit = 0; bit < BKLMElementNumbers::getMaximalLayerNumber(); ++bit) {
    if (pattern & (1 << bit)) {
      hits++;
    }
  }
  return hits;
}

unsigned int KLMMuidLikelihood::getTotalEndcapHits() const
{
  unsigned int hits = 0;
  unsigned int pattern = m_HitLayerPattern;
  for (int bit = BKLMElementNumbers::getMaximalLayerNumber();
       bit < BKLMElementNumbers::getMaximalLayerNumber() + EKLMElementNumbers::getMaximalLayerNumber(); ++bit) {
    if (pattern & (1 << bit)) {
      hits++;
    }
  }
  return hits;
}

bool Muid::isExtrapolatedBarrelLayerCrossed(unsigned int layer) const
{
  if (layer >= BKLMElementNumbers::getMaximalLayerNumber())
    return false;
  return m_ExtLayerPattern & (1 << layer);
};

bool Muid::isExtrapolatedEndcapLayerCrossed(unsigned int layer) const
{
  if (layer >= EKLMElementNumbers::getMaximalLayerNumber())
    return false;
  return m_ExtLayerPattern & (1 << (BKLMElementNumbers::getMaximalLayerNumber() + layer));
};
