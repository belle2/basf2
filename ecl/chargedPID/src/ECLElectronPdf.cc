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

#include <ecl/chargedPID/ECLElectronPdf.h>
#include <ecl/chargedPID/ParameterMap.h>

#include <cmath>
#include <TMath.h>

using namespace Belle2;
using namespace ECL;

void ECLElectronPdf::init(const char* parametersFileName)
{

  bool isAntiPart = (std::string(parametersFileName).find("anti") != std::string::npos);
  std::string chargePrefix = (!isAntiPart) ? "" : "anti";

  ParameterMap map(parametersFileName);
  ECLAbsPdf::init(map);

  unsigned int len = n_p_bins * n_theta_bins;

  m_params    = std::vector<Parameters>(len);
  m_integral1 = std::vector<double>(len);
  m_integral2 = std::vector<double>(len);

  unsigned int i(len - 1);

  for (unsigned int ip(0); ip < n_p_bins; ++ip) {

    for (unsigned int ith(0); ith < n_theta_bins; ++ith) {

      i = index(ip, ith); // global index in linearised (p,theta) matrix

      Parameters& prm = m_params[i];

      // Build Gaussian PDF
      prm.mu1 = map.param(name((chargePrefix + "electrons_mu1_").c_str(), ip, ith));
      prm.sigma1 = map.param(name((chargePrefix + "electrons_sigma1_").c_str(), ip, ith));
      prm.fraction = map.param(name((chargePrefix + "electrons_fraction_").c_str(), ip, ith));

      // Gaus normalisation (E/p in [0,+infty])
      m_integral1[i] = 0.5 * (1 - TMath::Erf(- prm.mu1 / prm.sigma1 / s_sqrt2));

      // Build Crystal ball PDF
      double mCB       = prm.mu2 = map.param(name((chargePrefix + "electrons_mu2_").c_str(), ip, ith));
      double sigmaCB   = prm.sigma2 = map.param(name((chargePrefix + "electrons_sigma2_").c_str(), ip, ith));
      double alphaCB   = prm.alpha = map.param(name((chargePrefix + "electrons_alpha_").c_str(), ip, ith));
      double nCB       = prm.nn = map.param(name((chargePrefix + "electrons_nn_").c_str(), ip, ith));

      // CB normalisation (E/p in [0,+infty])
      double tmin = - mCB / sigmaCB;  // ( E/p - m0 ) / sigma for E/p = 0
      double absalphaCB = TMath::Abs(alphaCB);
      if (tmin >= -absalphaCB) {
        m_integral2[i] = 0.5 * (1 - TMath::Erf(- mCB / sigmaCB / s_sqrt2));
      } else {
        double a = TMath::Power(nCB / absalphaCB, nCB) * std::exp(-0.5 * absalphaCB * absalphaCB);
        double b = nCB / absalphaCB - absalphaCB;
        double term1 = a * sigmaCB / (1.0 - nCB) * (1.0 / (TMath::Power(b - tmin, nCB - 1.0)) - 1.0 / (TMath::Power(nCB / absalphaCB,
                                                    nCB - 1.0)));
        double term2 = sigmaCB * s_sqrtPiOver2 * (1 - TMath::Erf(-absalphaCB / s_sqrt2));
        m_integral2[i] = (term1 + term2);
      }
    }

  }

}

double ECLElectronPdf::pdffunc(const double& eop, unsigned int i) const
{

  const Parameters& prm = m_params[i];

  double pdfgaus = TMath::Gaus(eop, prm.mu1, prm.sigma1, true) / m_integral1[i];

  double pdfCB;
  double mCB = prm.mu2;
  double sigmaCB = prm.sigma2;
  double alphaCB = prm.alpha;
  double nCB = prm.nn;
  double t = (eop - mCB) / sigmaCB;
  double absalphaCB = TMath::Abs(alphaCB);

  if (t >= -absalphaCB) {
    pdfCB = std::exp(-0.5 * t * t);
  } else {
    double a = TMath::Power(nCB / absalphaCB, nCB) * TMath::Exp(-0.5 * absalphaCB * absalphaCB);
    double b = nCB / absalphaCB - absalphaCB;
    pdfCB = a / TMath::Power(b - t, nCB);
  }
  pdfCB /= m_integral2[i];

  return prm.fraction * pdfgaus + (1 - prm.fraction) * pdfCB;

}
