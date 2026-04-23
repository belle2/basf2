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
  m_n_nodes{24, 24, 24, 24, 1}, m_trained(false), m_target_vars(1), m_output_scale{  0., 1.}
{
  m_weights.assign(n_weights_cal(), 0.);
  m_bias.assign(n_bias_cal(), 0.);
}

GRLMLP::GRLMLP(std::vector<unsigned short>& nodes,
               unsigned short targets,
               const std::vector<float>& outputscale
              ):
  m_n_nodes(nodes), m_trained(false), m_target_vars(targets), m_output_scale(outputscale)
{
  m_weights.assign(n_weights_cal(), 0.);
  m_bias.assign(n_bias_cal(), 0.);
}

unsigned
GRLMLP::n_weights_cal() const
{
  unsigned n_weights = 0;
  if (get_number_of_layers() > 1) {
    n_weights = m_n_nodes[0] * m_n_nodes[1];
    for (unsigned il = 1; il < get_number_of_layers() - 1; ++il) {
      n_weights += m_n_nodes[il] * m_n_nodes[il + 1];
    }
  }
  return n_weights;
}

unsigned
GRLMLP::n_bias_cal() const
{
  unsigned nbias = 0;
  if (get_number_of_layers() > 1) {
    for (unsigned il = 1; il < get_number_of_layers(); ++il) {
      nbias += m_n_nodes[il] ;
    }
  }
  return nbias;
}

