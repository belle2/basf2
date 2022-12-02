/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/grl/dataobjects/GRLMLP.h>
#include <cmath>

using namespace Belle2;

GRLMLP::GRLMLP():
  m_nNodes{23, 23, 1}, m_trained(false), m_targetVars(1), m_outputScale{ -1., 1.}
{
  m_weights.assign(nWeightsCal(), 0.);
}

GRLMLP::GRLMLP(std::vector<unsigned short>& nodes,
               unsigned short targets,
               const std::vector<float>& outputscale
              ):
  m_nNodes(nodes), m_trained(false), m_targetVars(targets), m_outputScale(outputscale)
{
  m_weights.assign(nWeightsCal(), 0.);
}

unsigned
GRLMLP::nWeightsCal() const
{
  unsigned nWeights = 0;
  if (getNumberOfLayers() > 1) {
    nWeights = (m_nNodes[0] + 1) * m_nNodes[1];
    for (unsigned il = 1; il < getNumberOfLayers() - 1; ++il) {
      nWeights += (m_nNodes[il] + 1) * m_nNodes[il + 1];
    }
  }
  return nWeights;
}

