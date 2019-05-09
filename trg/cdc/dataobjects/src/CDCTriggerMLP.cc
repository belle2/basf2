#include <trg/cdc/dataobjects/CDCTriggerMLP.h>
#include <cmath>

using namespace Belle2;

CDCTriggerMLP::CDCTriggerMLP():
  nNodes{27, 27, 2}, trained(false), targetVars(3), outputScale{ -1., 1., -1., 1.},
  phiRange{0., 2. * M_PI}, invptRange{ -5., 5.}, thetaRange{0., M_PI},
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
  <<< <<< < HEAD
  T0fromHits(false),
  et_option("fastestpriority")
  == == == =
    et_option("etf_or_fastestpriority"),
    T0fromHits(false)
    >>>>>>> dc2435e7b8b78699d6e3cacd7ae844cf70a1fcdb {
  weights.assign(nWeightsCal(), 0.);
}

CDCTriggerMLP::CDCTriggerMLP(std::vector<unsigned short> & nodes,
                             unsigned short targets,
                             std::vector<float> & outputscale,
                             std::vector<float> & phirange,
                             std::vector<float> & invptrange,
                             std::vector<float> & thetarange,
                             unsigned short maxHits,
                             unsigned long pattern,
                             unsigned long patternMask,
                             unsigned short tmax,
                             bool calcT0,
                             std::string  etoption):
  nNodes(nodes), trained(false), targetVars(targets), outputScale(outputscale),
  phiRange(phirange), invptRange(invptrange), thetaRange(thetarange),
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
  et_option(etoption),
  T0fromHits(calcT0)
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
CDCTriggerMLP::inPhiRange(float phi) const
{
  return ((phiRange[0] <= (phi - 2. * M_PI) && (phi - 2. * M_PI) <= phiRange[1]) ||
          (phiRange[0] <= phi && phi <= phiRange[1]) ||
          (phiRange[0] <= (phi + 2. * M_PI) && (phi + 2. * M_PI) <= phiRange[1]));
}

bool
CDCTriggerMLP::inPtRange(float pt) const
{
  return (invptRange[0] <= 1. / pt && 1. / pt <= invptRange[1]);
}

bool
CDCTriggerMLP::inInvptRange(float invpt) const
{
  return (invptRange[0] <= invpt && invpt <= invptRange[1]);
}

bool
CDCTriggerMLP::inThetaRange(float theta) const
{
  return (thetaRange[0] <= theta && theta <= thetaRange[1]);
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
