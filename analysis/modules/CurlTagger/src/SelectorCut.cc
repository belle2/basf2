/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/CurlTagger/SelectorCut.h>

//Root includes
#include "TMath.h"

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

SelectorCut::~SelectorCut() = default;

std::vector<float> SelectorCut::getVariables(Particle* iPart, Particle* jPart)
{
  float chargeProduct = iPart -> getCharge() * jPart -> getCharge();
  float magDiffP = (iPart->getMomentum() - jPart->getMomentum()).R();
  float phi = acos(iPart->getMomentum().Unit().Dot(jPart->getMomentum().Unit()));
  return {chargeProduct, magDiffP, phi};
}

float SelectorCut::getResponse(Particle* iPart, Particle* jPart)
{
  //Selection from BN1079
  std::vector<float> variables = getVariables(iPart, jPart);
  float chargeProduct = variables[0];
  float magDiffP = variables[1];
  float phi = variables[2];

  if (magDiffP > m_magDiffPCut) {return 0.;}
  if (chargeProduct > 0) {
    if (phi < m_sameChargePhiCut * TMath::Pi() / 180) {return 1.;}
  }
  if (chargeProduct < 0) {
    if (phi > m_oppositeChargePhiCut * TMath::Pi() / 180) {return 1.;}
  }

  return 0.;
}
