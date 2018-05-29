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

#include <ecl/chargedPID/ECLAbsPdf.h>
#include <framework/gearbox/Unit.h>
#include <TMath.h>
#include <TF1.h>

using namespace Belle2;
using namespace ECL;

double ECLAbsPdf::s_energy_unit = Unit::GeV;
double ECLAbsPdf::s_ang_unit = Unit::rad;

unsigned int ECLAbsPdf::index(const double& p, const double& theta) const
{

  unsigned int ip(m_n_p_bins - 1);

  for (int i(ip); i >= 0; --i) {
    if (p > m_p_min[i] * s_energy_unit) {
      ip = static_cast<unsigned int>(i);
      break;
    }
  }
  unsigned int ith(m_n_theta_bins - 1);
  for (int i(ith); i >= 0; --i) {
    if (TMath::Abs(theta) > m_theta_min[i] * s_ang_unit) {
      ith = static_cast<unsigned int>(i);
      break;
    }
  }

  return index(ip, ith);

}

void ECLAbsPdf::init(const ParameterMap& map)
{

  m_n_theta_bins = map.param("n_theta_bins");
  m_n_p_bins     = map.param("n_p_bins");

  m_vars = std::vector<RooRealVar>(m_n_theta_bins * m_n_p_bins);
  m_PDFs = std::vector<RooAddPdf>(m_n_theta_bins * m_n_p_bins);

  m_theta_min = new double[m_n_theta_bins - 1];
  m_p_min     = new double[m_n_p_bins - 1];

  for (unsigned int i(0); i < m_n_theta_bins; ++i) {
    std::ostringstream nm;
    nm << "theta_" << i << "_min";
    m_theta_min[i] = map.param(nm.str());
  }
  for (unsigned int i(0); i < m_n_p_bins; ++i) {
    std::ostringstream nm;
    nm << "p_" << i << "_min";
    m_p_min[i] = map.param(nm.str());
  }

}

double ECLAbsPdf::pdffunc(const double& eop, unsigned int i) const
{

  TF1* pdf = (TF1*) m_PDFs[i].asTF(RooArgList(m_vars[i]), *(m_PDFs[i].getParameters(m_vars[i])), RooArgSet(m_vars[i]));
  return pdf->Eval(eop);

}
