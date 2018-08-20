/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marcel Hohmann                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/CurlTagger/SelectorCut.h>

using namespace Belle2;
using namespace CurlTagger;

SelectorCut::SelectorCut(bool belleFlag)
{
  if (belleFlag) {
    //BN1079 cuts
    m_magDiffPCut = 0.1;
    m_sameChargePhiCut = 15;
    m_oppositeChargePhiCut = 165;
  } else {
    // TODO - update these with actual values. Is this even worth if switching to MVA anyway?
    m_magDiffPCut = 0.1;
    m_sameChargePhiCut = 15;
    m_oppositeChargePhiCut = 165;
  }

}

SelectorCut::~SelectorCut()
{
}

std::vector<float> SelectorCut::getVariables(Particle* iPart, Particle* jPart)
{
  float chargeMult = iPart -> getCharge() * jPart -> getCharge();
  float magDiffP = (iPart->getMomentum() - jPart->getMomentum()).Mag();
  float phi = iPart->getMomentum().Angle(jPart->getMomentum());
  return {chargeMult, magDiffP, phi};
}

float SelectorCut::getProbability(Particle* iPart, Particle* jPart)
{
  //Selection from BN1079
  std::vector<float> variables = getVariables(iPart, jPart);
  float chargeMult = variables[0];
  float magDiffP = variables[1];
  float phi = variables[2];

  if (magDiffP > m_magDiffPCut) {return 0.;}
  if (chargeMult > 0) {
    if (phi < m_sameChargePhiCut * TMath::Pi() / 180) {return 1.;}
  }
  if (chargeMult < 0) {
    if (phi > m_oppositeChargePhiCut * TMath::Pi() / 180) {return 1.;}
  }

  return 0.;
}
