/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Class for proton ECL E/p PDFs                                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/chargedPID/ECLProtonPdf.h>
#include <ecl/chargedPID/ParameterMap.h>

#include <RooArgList.h>
#include <RooArgSet.h>
#include <RooRealVar.h>
#include <RooBifurGauss.h>
#include <RooGaussian.h>
#include <RooAddPdf.h>

using namespace Belle2;
using namespace ECL;

void ECLProtonPdf::init(const char* parametersFileName)
{

  bool isAntiPart = (std::string(parametersFileName).find("anti") != std::string::npos);
  std::string chargePrefix = (!isAntiPart) ? "" : "anti";

  ParameterMap map(parametersFileName);
  ECLAbsPdf::init(map);

  unsigned int len = m_n_p_bins * m_n_theta_bins;

  m_params            = std::vector<Parameters>(len);
  m_muonlike.m_params = std::vector<ECLMuonPdf::Parameters>(len);

  ECLAbsPdf& abs = m_muonlike;
  abs.init(map);

  unsigned int i(len - 1);

  for (unsigned int ip(0); ip < m_n_p_bins; ++ip) {

    for (unsigned int ith(0); ith < m_n_theta_bins; ++ith) {

      i = index(ip, ith);

      Parameters& p_prm = m_params[i];

      // Create RooRealVar representing the fitted observable.
      p_prm.fitrange_dn = map.param(name((chargePrefix + "protons_fit_range_dn_").c_str(), ip, ith));
      p_prm.fitrange_up = map.param(name((chargePrefix + "protons_fit_range_up_").c_str(), ip, ith));
      RooRealVar var("eop", "E/P (c)", p_prm.fitrange_dn, p_prm.fitrange_up);

      // Build Gaussian PDF
      p_prm.mu3      = map.param(name((chargePrefix + "protons_mu3_").c_str(), ip, ith));
      p_prm.sigma3   = map.param(name((chargePrefix + "protons_sigma3_").c_str(), ip, ith));

      RooRealVar gaus_mu("#mu_{g}", "mean of gaussian", p_prm.mu3);
      RooRealVar gaus_sigma("#sigma_{g}", "width of gaussian", p_prm.sigma3);
      RooGaussian gaus("gaus", "gaussian PDF", var, gaus_mu, gaus_sigma);

      // Build muon-like PDF (Bifurcated Gaussian+Gaussian)
      ECLMuonPdf::Parameters& mu_prm = m_muonlike.m_params[i];

      mu_prm.mu1      = map.param(name((chargePrefix + "protons_mu1_").c_str(), ip, ith));
      mu_prm.sigma1l  = map.param(name((chargePrefix + "protons_sigma1l_").c_str(), ip, ith));
      mu_prm.sigma1r  = map.param(name((chargePrefix + "protons_sigma1r_").c_str(), ip, ith));
      mu_prm.fraction = map.param(name((chargePrefix + "protons_fraction1_").c_str(), ip, ith));
      mu_prm.mu2      = map.param(name((chargePrefix + "protons_mu2_").c_str(), ip, ith));
      mu_prm.sigma2   = map.param(name((chargePrefix + "protons_sigma2_").c_str(), ip, ith));

      RooRealVar mu_bifurgaus_mu("#mu_{mu_bg}", "mean of bifurcated gaussian", mu_prm.mu1);
      RooRealVar mu_bifurgaus_sigmal("#sigma_{mu_bg}_{L}", "widthL of bifurcated gaussian", mu_prm.sigma1l);
      RooRealVar mu_bifurgaus_sigmar("#sigma_{mu_bg}_{R}", "widthR of bifurcated gaussian", mu_prm.sigma1r);
      RooRealVar mu_bifurgaus_frac("frac_{mu_bg}", "bifurcated gaussian fraction", mu_prm.fraction);
      RooBifurGauss mu_bifurgaus("mu_bifurgaus", "bifurcated  gaussian PDF", var, mu_bifurgaus_mu, mu_bifurgaus_sigmal,
                                 mu_bifurgaus_sigmar);

      RooRealVar mu_gaus_mu("#mu_{mu_g}", "mean of gaussian", mu_prm.mu2);
      RooRealVar mu_gaus_sigma("#sigma_{mu_g}", "width of gaussian", mu_prm.sigma2);
      RooGaussian mu_gaus("mu_gaus", "gaussian PDF", var, mu_gaus_mu, mu_gaus_sigma);

      // Combine PDFs
      RooAddPdf pdf1("pdf1", "bifurcated gaussian + gaussian", mu_bifurgaus, mu_gaus, mu_bifurgaus_frac);

      p_prm.fraction = map.param(name((chargePrefix + "protons_fraction2_").c_str(), ip, ith));
      RooRealVar pdf1_frac("frac_{pdf1}", "pdf1 fraction", p_prm.fraction);

      RooAddPdf pdf2("pdf2", "pdf1 + gaussian", pdf1, gaus, pdf1_frac);

      TF1* func = (TF1*) pdf2.asTF(RooArgList(var), *(pdf2.getParameters(var)), RooArgSet(var))->Clone();
      m_PDFs[i] = func;

    }
  }
}
