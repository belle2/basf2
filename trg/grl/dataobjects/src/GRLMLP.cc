#include <trg/grl/dataobjects/GRLMLP.h>
#include <cmath>

using namespace Belle2;

GRLMLP::GRLMLP():
  nNodes{23, 23, 1}, trained(false), targetVars(1), outputScale{ -1., 1.}
{
  weights.assign(nWeightsCal(), 0.);
}

GRLMLP::GRLMLP(std::vector<unsigned short>& nodes,
               unsigned short targets,
               const std::vector<float>& outputscale
              ):
  nNodes(nodes), trained(false), targetVars(targets), outputScale(outputscale)
{
  weights.assign(nWeightsCal(), 0.);
}

unsigned
GRLMLP::nWeightsCal() const
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

