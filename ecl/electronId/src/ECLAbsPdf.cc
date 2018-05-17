/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Base class for ECL E/p PDFs                                            *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/electronId/ECLAbsPdf.h>
#include <framework/gearbox/Unit.h>
#include <TMath.h>

using namespace Belle2;
using namespace ECL;

double ECLAbsPdf::s_energy_unit = Unit::GeV;
double ECLAbsPdf::s_ang_unit = Unit::rad;

unsigned int ECLAbsPdf::index(const double& p, const double& theta) const
{

  unsigned int ip(n_p_bins - 1);

  for (int i(ip); i >= 0; --i) {
    if (p > p_min[i] * s_energy_unit) {
      ip = static_cast<unsigned int>(i);
      break;
    }
  }
  unsigned int ith(n_theta_bins - 1);
  for (int i(ith); i >= 0; --i) {
    if (TMath::Abs(theta) > theta_min[i] * s_ang_unit) {
      ith = static_cast<unsigned int>(i);
      break;
    }
  }

  return index(ip, ith);

}

void ECLAbsPdf::init(const ParameterMap& map)
{

  n_theta_bins = map.param("n_theta_bins");
  n_p_bins     = map.param("n_p_bins");

  theta_min = new double[n_theta_bins - 1];
  p_min     = new double[n_p_bins - 1];

  for (unsigned int i(0); i < n_theta_bins; ++i) {
    std::ostringstream nm;
    nm << "theta_" << i << "_min";
    theta_min[i] = map.param(nm.str());
  }
  for (unsigned int i(0); i < n_p_bins; ++i) {
    std::ostringstream nm;
    nm << "p_" << i << "_min";
    p_min[i] = map.param(nm.str());
  }

}
