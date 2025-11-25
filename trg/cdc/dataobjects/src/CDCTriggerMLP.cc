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
  nNodes{27, 27, 2}, trained(false), targetVars(3), outputScale{ -1., 1., -1., 1.},
  phiRangeUse{0., 2. * M_PI}, invptRangeUse{ -5., 5.}, thetaRangeUse{0., M_PI},
  phiRangeTrain{0., 2. * M_PI}, invptRangeTrain{ -5., 5.}, thetaRangeTrain{0., M_PI},
  maxHitsPerSL(1), SLpattern(0), SLpatternMask(0), tMax(256),
  relevantID{ -1., 1.,
              -10., 1.,
              -1., 1.,
              -1., 10.,
              -1., 1.,
              -10.5, 1.,
              -1., 1.,
              -1., 11.,
              -1., 1.},
    et_option("etf_or_fastestpriority")
{
  weights.assign(nWeightsCal(), 0.);
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
  nNodes(nodes), trained(false), targetVars(targets), outputScale(outputscale),
  phiRangeUse(phirangeUse), invptRangeUse(invptrangeUse), thetaRangeUse(thetarangeUse),
  phiRangeTrain(phirangeTrain), invptRangeTrain(invptrangeTrain), thetaRangeTrain(thetarangeTrain),
  maxHitsPerSL(maxHits), SLpattern(pattern), SLpatternMask(patternMask),
  tMax(tmax),
  relevantID{ -1., 1.,
              -10., 1.,
              -1., 1.,
              -1., 10.,
              -1., 1.,
              -10.5, 1.,
              -1., 1.,
              -1., 11.,
              -1., 1.},
    et_option(etoption)
{
  weights.assign(nWeightsCal(), 0.);
}

unsigned
CDCTriggerMLP::nWeightsCal() const
{
  unsigned nWeights = 0;
  if (nLayers() > 1) {
    nWeights = (nNodes[0] + 1) * nNodes[1];
    for (unsigned il = 1; il < nLayers() - 1; ++il) {
      nWeights += (nNodes[il] + 1) * nNodes[il + 1];
    }
  }
  return nWeights;
}

bool
CDCTriggerMLP::inPhiRangeUse(float phi) const
{
  return ((phiRangeUse[0] <= (phi - 2. * M_PI) && (phi - 2. * M_PI) <= phiRangeUse[1]) ||
          (phiRangeUse[0] <= phi && phi <= phiRangeUse[1]) ||
          (phiRangeUse[0] <= (phi + 2. * M_PI) && (phi + 2. * M_PI) <= phiRangeUse[1]));
}

bool
CDCTriggerMLP::inPtRangeUse(float pt) const
{
  return (invptRangeUse[0] <= 1. / pt && 1. / pt <= invptRangeUse[1]);
}

bool
CDCTriggerMLP::inInvptRangeUse(float invpt) const
{
  return (invptRangeUse[0] <= invpt && invpt <= invptRangeUse[1]);
}

bool
CDCTriggerMLP::inThetaRangeUse(float theta) const
{
  return (thetaRangeUse[0] <= theta && theta <= thetaRangeUse[1]);
}

bool
CDCTriggerMLP::inPhiRangeTrain(float phi) const
{
  return ((phiRangeTrain[0] <= (phi - 2. * M_PI) && (phi - 2. * M_PI) <= phiRangeTrain[1]) ||
          (phiRangeTrain[0] <= phi && phi <= phiRangeTrain[1]) ||
          (phiRangeTrain[0] <= (phi + 2. * M_PI) && (phi + 2. * M_PI) <= phiRangeTrain[1]));
}

bool
CDCTriggerMLP::inPtRangeTrain(float pt) const
{
  return (invptRangeTrain[0] <= 1. / pt && 1. / pt <= invptRangeTrain[1]);
}

bool
CDCTriggerMLP::inInvptRangeTrain(float invpt) const
{
  return (invptRangeTrain[0] <= invpt && invpt <= invptRangeTrain[1]);
}

bool
CDCTriggerMLP::inThetaRangeTrain(float theta) const
{
  return (thetaRangeTrain[0] <= theta && theta <= thetaRangeTrain[1]);
}

bool
CDCTriggerMLP::isRelevant(float relId, unsigned iSL) const
{
  return (relevantID[2 * iSL] <= relId && relId <= relevantID[2 * iSL + 1]);
}

float
CDCTriggerMLP::scaleId(double relId, unsigned iSL) const
{
  float scale = 2. / (relevantID[2 * iSL + 1] - relevantID[2 * iSL]);
  // round down to nearest power of 2
  scale = pow(2, floor(log2(scale)));
  float offset = (relevantID[2 * iSL] + relevantID[2 * iSL + 1]) / 2.;
  return scale * (relId - offset);
}

std::vector<float>
CDCTriggerMLP::scaleTarget(std::vector<float> target) const
{
  std::vector<float> scaled;
  scaled.assign(target.size(), 0.);
  for (unsigned i = 0; i < target.size(); ++i) {
    scaled[i] = 2. * (target[i] - outputScale[2 * i]) / (outputScale[2 * i + 1] - outputScale[2 * i]) - 1.;
  }
  return scaled;
}

std::vector<float>
CDCTriggerMLP::unscaleTarget(std::vector<float> target) const
{
  std::vector<float> unscaled;
  unscaled.assign(target.size(), 0.);
  for (unsigned i = 0; i < target.size(); ++i) {
    unscaled[i] = (target[i] + 1.) * (outputScale[2 * i + 1] - outputScale[2 * i]) / 2. + outputScale[2 * i];
  }
  return unscaled;
}

int
CDCTriggerMLP::zIndex() const
{
  return (targetVars & 1) ? 0 : -1;
}

int
CDCTriggerMLP::thetaIndex() const
{
  return (targetVars & 2) ? (targetVars & 1) : -1;
}
