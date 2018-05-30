/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Class for electron ECL E/p PDFs                                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/chargedPID/ECLElectronPdf.h>
#include <ecl/chargedPID/ParameterMap.h>

#include <RooArgList.h>
#include <RooArgSet.h>
#include <RooRealVar.h>
#include <RooCBShape.h>
#include <RooGaussian.h>
#include <RooAddPdf.h>

using namespace Belle2;
using namespace ECL;

void ECLElectronPdf::init(const char* parametersFileName)
{

  bool isAntiPart = (std::string(parametersFileName).find("anti") != std::string::npos);
  std::string chargePrefix = (!isAntiPart) ? "" : "anti";

  ParameterMap map(parametersFileName);
  ECLAbsPdf::init(map);

  unsigned int len = m_n_p_bins * m_n_theta_bins;

  m_params = std::vector<Parameters>(len);

  unsigned int i(len - 1);

  for (unsigned int ip(0); ip < m_n_p_bins; ++ip) {

    for (unsigned int ith(0); ith < m_n_theta_bins; ++ith) {

      i = index(ip, ith); // global index in linearised (p,theta) matrix

      Parameters& prm = m_params[i];

      // Create RooRealVar representing the fitted observable.
      prm.fitrange_dn = map.param(name((chargePrefix + "electrons_fit_range_dn_").c_str(), ip, ith));
      prm.fitrange_up = map.param(name((chargePrefix + "electrons_fit_range_up_").c_str(), ip, ith));
      RooRealVar var("eop", "E/P (c)", prm.fitrange_dn, prm.fitrange_up);

      // Build Gaussian PDF
      prm.mu1      = map.param(name((chargePrefix + "electrons_mu1_").c_str(), ip, ith));
      prm.sigma1   = map.param(name((chargePrefix + "electrons_sigma1_").c_str(), ip, ith));
      prm.fraction = map.param(name((chargePrefix + "electrons_fraction_").c_str(), ip, ith));

      RooRealVar gaus_mu("#mu_{g}", "mean of gaussian", prm.mu1);
      RooRealVar gaus_sigma("#sigma_{g}", "width of gaussian", prm.sigma1);
      RooRealVar gaus_frac("frac_{g}", "gaussian fraction", prm.fraction);
      RooGaussian gaus("gaus", "gaussian PDF", var, gaus_mu, gaus_sigma);

      // Build Crystal ball PDF
      prm.mu2    = map.param(name((chargePrefix + "electrons_mu2_").c_str(), ip, ith));
      prm.sigma2 = map.param(name((chargePrefix + "electrons_sigma2_").c_str(), ip, ith));
      prm.alpha  = map.param(name((chargePrefix + "electrons_alpha_").c_str(), ip, ith));
      prm.nn     = map.param(name((chargePrefix + "electrons_nn_").c_str(), ip, ith));

      RooRealVar cb_mu("#mu_{CB}", "mean of CB", prm.mu2);
      RooRealVar cb_sigma("#sigma_{CB}", "width of CB shape", prm.sigma2);
      RooRealVar cb_alpha("#alpha_{CB}", "tail length", prm.alpha);
      RooRealVar cb_nn("nn_{CB}", "tail slope", prm.nn);
      RooCBShape cb("CB", "crystal ball PDF", var, cb_mu, cb_sigma, cb_alpha, cb_nn);

      // Combine PDFs
      RooAddPdf pdf("pdf", "gaussian + CB PDF", gaus, cb, gaus_frac);

      TF1* func = (TF1*) pdf.asTF(RooArgList(var), *(pdf.getParameters(var)), RooArgSet(var))->Clone();
      m_PDFs[i] = func;

    }

  }

}
