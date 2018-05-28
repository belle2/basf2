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

#include <TF1.h>
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

  unsigned int len = n_p_bins * n_theta_bins;

  m_params = std::vector<Parameters>(len);
  m_vars   = std::vector<RooRealVar>(len);
  m_PDFs   = std::vector<RooAddPdf>(len);

  unsigned int i(len - 1);

  for (unsigned int ip(0); ip < n_p_bins; ++ip) {

    for (unsigned int ith(0); ith < n_theta_bins; ++ith) {

      i = index(ip, ith); // global index in linearised (p,theta) matrix

      Parameters& prm = m_params[i];

      // Create RooRealVar representing the fitted observable.
      prm.fitrange_dn = map.param(name((chargePrefix + "electrons_fit_range_dn_").c_str(), ip, ith));
      prm.fitrange_up = map.param(name((chargePrefix + "electrons_fit_range_up_").c_str(), ip, ith));
      RooRealVar var("eop", "E/P (c)", prm.fitrange_dn, prm.fitrange_up);
      m_vars[i] = var;

      // Build Gaussian PDF
      prm.mu1      = map.param(name((chargePrefix + "electrons_mu1_").c_str(), ip, ith));
      prm.sigma1   = map.param(name((chargePrefix + "electrons_sigma1_").c_str(), ip, ith));
      prm.fraction = map.param(name((chargePrefix + "electrons_fraction_").c_str(), ip, ith));

      RooRealVar gaus_mu("#mu_{1}", "mean of gaussian", prm.mu1);
      RooRealVar gaus_sigma("#sigma_{1}", "width of gaussian", prm.sigma1);
      RooRealVar gaus_frac("frac", "gaussian fraction", prm.fraction);
      RooGaussian gaus("gaus", "gaussian PDF", var, gaus_mu, gaus_sigma);

      // Build Crystal ball PDF
      prm.mu2 = map.param(name((chargePrefix + "electrons_mu2_").c_str(), ip, ith));
      prm.sigma2 = map.param(name((chargePrefix + "electrons_sigma2_").c_str(), ip, ith));
      prm.alpha = map.param(name((chargePrefix + "electrons_alpha_").c_str(), ip, ith));
      prm.nn = map.param(name((chargePrefix + "electrons_nn_").c_str(), ip, ith));

      RooRealVar cb_mu("#mu_{2}", "mean of CB", prm.mu2);
      RooRealVar cb_sigma("#sigma_{2}", "width of CB shape", prm.sigma2);
      RooRealVar cb_alpha("#alpha", "tail length", prm.alpha);
      RooRealVar cb_nn("nn", "tail slope", prm.nn);
      RooCBShape cb("cb", "crystal ball PDF", var, cb_mu, cb_sigma, cb_alpha, cb_nn);

      // Combine PDFs
      RooAddPdf pdf("pdf", "gaussian + CB PDF", gaus, cb, gaus_frac);
      m_PDFs[i] = pdf;

    }

  }

}

double ECLElectronPdf::pdffunc(const double& eop, unsigned int i) const
{

  TF1* pdf = (TF1*) m_PDFs[i].asTF(RooArgList(m_vars[i]), *(m_PDFs[i].getParameters(m_vars[i])), RooArgSet(m_vars[i]));
  return pdf->Eval(eop);

}
