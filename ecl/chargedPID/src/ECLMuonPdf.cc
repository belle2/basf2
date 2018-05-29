/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Class for muon ECL E/p PDFs                                            *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/chargedPID/ECLMuonPdf.h>
#include <ecl/chargedPID/ParameterMap.h>

#include <RooRealVar.h>
#include <RooBifurGauss.h>
#include <RooGaussian.h>
#include <RooAddPdf.h>

using namespace Belle2;
using namespace ECL;

void ECLMuonPdf::init(const char* parametersFileName)
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

      i = index(ip, ith);

      Parameters& prm = m_params[i];

      // Create RooRealVar representing the fitted observable.
      prm.fitrange_dn = map.param(name((chargePrefix + "muons_fit_range_dn_").c_str(), ip, ith));
      prm.fitrange_up = map.param(name((chargePrefix + "muons_fit_range_up_").c_str(), ip, ith));
      RooRealVar var("eop", "E/P (c)", prm.fitrange_dn, prm.fitrange_up);
      m_vars[i] = var;

      // Build Bifurcated Gaussian PDF
      prm.mu1      = map.param(name((chargePrefix + "muons_mu1_").c_str(), ip, ith));
      prm.sigma1l  = map.param(name((chargePrefix + "muons_sigma1l_").c_str(), ip, ith));
      prm.sigma1r  = map.param(name((chargePrefix + "muons_sigma1r_").c_str(), ip, ith));
      prm.fraction = map.param(name((chargePrefix + "muons_fraction_").c_str(), ip, ith));

      RooRealVar bifurgaus_mu("#mu_{bg}", "mean of bifurcated gaussian", prm.mu1);
      RooRealVar bifurgaus_sigmal("#sigma_{bg}_{L}", "widthL of bifurcated gaussian", prm.sigma1l);
      RooRealVar bifurgaus_sigmar("#sigma_{bg}_{R}", "widthR of bifurcated gaussian", prm.sigma1r);
      RooRealVar bifurgaus_frac("frac_{bg}", "bifurcated gaussian fraction", prm.fraction);
      RooBifurGauss bifurgaus("bifurgaus", "bifurcated  gaussian PDF", var, bifurgaus_mu, bifurgaus_sigmal, bifurgaus_sigmar);

      // Build Gaussian PDF
      prm.mu2    = map.param(name((chargePrefix + "muons_mu2_").c_str(), ip, ith));
      prm.sigma2 = map.param(name((chargePrefix + "muons_sigma2_").c_str(), ip, ith));

      RooRealVar gaus_mu("#mu_{g}", "mean of gaussian", prm.mu2);
      RooRealVar gaus_sigma("#sigma_{g}", "width of gaussian", prm.sigma2);
      RooGaussian gaus("gaus", "gaussian PDF", var, gaus_mu, gaus_sigma);

      // Combine PDFs
      RooAddPdf pdf("pdf", "bifurcated gaussian + gaussian", bifurgaus, gaus, bifurgaus_frac);
      m_PDFs[i] = pdf;

    }

  }

}
