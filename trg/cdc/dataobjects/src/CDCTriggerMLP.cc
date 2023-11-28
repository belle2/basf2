/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <trg/cdc/dataobjects/CDCTriggerMLP.h>
#include <cmath>

using namespace Belle2;

CDCTriggerMLP::CDCTriggerMLP():
  m_nNodes{27, 27, 2}, m_trained(false), m_targetVars(3), m_outputScale{ -1., 1., -1., 1.},
  m_phiRangeUse{0., 2. * M_PI}, m_invptRangeUse{ -5., 5.}, m_thetaRangeUse{0., M_PI},
  m_phiRangeTrain{0., 2. * M_PI}, m_invptRangeTrain{ -5., 5.}, m_thetaRangeTrain{0., M_PI},
  m_maxHitsPerSL(1), m_SLpattern(0), m_SLpatternMask(0), m_tMax(256),
  m_relevantID{ -1., 1.,
                -10., 1.,
                -1., 1.,
                -1., 10.,
                -1., 1.,
                -10.5, 1.,
                -1., 1.,
                -1., 11.,
                -1., 1.},
    m_et_option("etf_or_fastestpriority")
{
  m_weights.assign(nWeightsCal(), 0.);
}

CDCTriggerMLP::CDCTriggerMLP(std::vector<unsigned short>& nodes,
                             unsigned short targets,
                             std::vector<float>& outputscale,
                             std::vector<float>& phirangeUse,
                             std::vector<float>& invptrangeUse,
                             std::vector<float>& thetarangeUse,
                             std::vector<float>& phirangeTrain,
                             std::vector<float>& invptrangeTrain,
                             std::vector<float>& thetarangeTrain,
                             unsigned short maxHits,
                             unsigned long pattern,
                             unsigned long patternMask,
                             unsigned short tmax,
                             const std::string&  etoption):
  m_nNodes(nodes), m_trained(false), m_targetVars(targets), m_outputScale(outputscale),
  m_phiRangeUse(phirangeUse), m_invptRangeUse(invptrangeUse), m_thetaRangeUse(thetarangeUse),
  m_phiRangeTrain(phirangeTrain), m_invptRangeTrain(invptrangeTrain), m_thetaRangeTrain(thetarangeTrain),
  m_maxHitsPerSL(maxHits), m_SLpattern(pattern), m_SLpatternMask(patternMask),
  m_tMax(tmax),
  m_relevantID{ -1., 1.,
                -10., 1.,
                -1., 1.,
                -1., 10.,
                -1., 1.,
                -10.5, 1.,
                -1., 1.,
                -1., 11.,
                -1., 1.},
    m_et_option(etoption)
{
  m_weights.assign(nWeightsCal(), 0.);
}

unsigned
CDCTriggerMLP::nWeightsCal() const
{
  unsigned nWeights = 0;
  if (nLayers() > 1) {
    nWeights = (m_nNodes[0] + 1) * m_nNodes[1];
    for (unsigned il = 1; il < nLayers() - 1; ++il) {
      nWeights += (m_nNodes[il] + 1) * m_nNodes[il + 1];
    }
  }
  return nWeights;
}

bool
CDCTriggerMLP::inPhiRangeUse(float phi) const
{
  return ((m_phiRangeUse[0] <= (phi - 2. * M_PI) && (phi - 2. * M_PI) <= m_phiRangeUse[1]) ||
          (m_phiRangeUse[0] <= phi && phi <= m_phiRangeUse[1]) ||
          (m_phiRangeUse[0] <= (phi + 2. * M_PI) && (phi + 2. * M_PI) <= m_phiRangeUse[1]));
}

bool
CDCTriggerMLP::inPtRangeUse(float pt) const
{
  return (m_invptRangeUse[0] <= 1. / pt && 1. / pt <= m_invptRangeUse[1]);
}

bool
CDCTriggerMLP::inInvptRangeUse(float invpt) const
{
  return (m_invptRangeUse[0] <= invpt && invpt <= m_invptRangeUse[1]);
}

bool
CDCTriggerMLP::inThetaRangeUse(float theta) const
{
  return (m_thetaRangeUse[0] <= theta && theta <= m_thetaRangeUse[1]);
}

bool
CDCTriggerMLP::inPhiRangeTrain(float phi) const
{
  return ((m_phiRangeTrain[0] <= (phi - 2. * M_PI) && (phi - 2. * M_PI) <= m_phiRangeTrain[1]) ||
          (m_phiRangeTrain[0] <= phi && phi <= m_phiRangeTrain[1]) ||
          (m_phiRangeTrain[0] <= (phi + 2. * M_PI) && (phi + 2. * M_PI) <= m_phiRangeTrain[1]));
}

bool
CDCTriggerMLP::inPtRangeTrain(float pt) const
{
  return (m_invptRangeTrain[0] <= 1. / pt && 1. / pt <= m_invptRangeTrain[1]);
}

bool
CDCTriggerMLP::inInvptRangeTrain(float invpt) const
{
  return (m_invptRangeTrain[0] <= invpt && invpt <= m_invptRangeTrain[1]);
}

bool
CDCTriggerMLP::inThetaRangeTrain(float theta) const
{
  return (m_thetaRangeTrain[0] <= theta && theta <= m_thetaRangeTrain[1]);
}

bool
CDCTriggerMLP::isRelevant(float relId, unsigned iSL) const
{
  return (m_relevantID[2 * iSL] <= relId && relId <= m_relevantID[2 * iSL + 1]);
}

float
CDCTriggerMLP::scaleId(double relId, unsigned iSL) const
{
  float scale = 2. / (m_relevantID[2 * iSL + 1] - m_relevantID[2 * iSL]);
  // round down to nearest power of 2
  scale = pow(2, floor(log2(scale)));
  float offset = (m_relevantID[2 * iSL] + m_relevantID[2 * iSL + 1]) / 2.;
  return scale * (relId - offset);
}

std::vector<float>
CDCTriggerMLP::scaleTarget(std::vector<float> target) const
{
  std::vector<float> scaled;
  scaled.assign(target.size(), 0.);
  for (unsigned i = 0; i < target.size(); ++i) {
    scaled[i] = 2. * (target[i] - m_outputScale[2 * i]) / (m_outputScale[2 * i + 1] - m_outputScale[2 * i]) - 1.;
  }
  return scaled;
}

std::vector<float>
CDCTriggerMLP::unscaleTarget(std::vector<float> target) const
{
  std::vector<float> unscaled;
  unscaled.assign(target.size(), 0.);
  for (unsigned i = 0; i < target.size(); ++i) {
    unscaled[i] = (target[i] + 1.) * (m_outputScale[2 * i + 1] - m_outputScale[2 * i]) / 2. + m_outputScale[2 * i];
  }
  return unscaled;
}

int
CDCTriggerMLP::zIndex() const
{
  return (m_targetVars & 1) ? 0 : -1;
}

int
CDCTriggerMLP::thetaIndex() const
{
  return (m_targetVars & 2) ? (m_targetVars & 1) : -1;
}
