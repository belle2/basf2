#include <ecl/electronId/ECLPionPdf.h>
#include <ecl/electronId/ParameterMap.h>
#include <cmath>
#include <TMath.h>

using namespace Belle2;

void ECLPionPdf::init(const char* parametersFileName)
{

  ParameterMap map(parametersFileName);
  ECLAbsPdf::init(map);

  unsigned int len = n_p_bins * n_theta_bins;

  m_params = std::vector<Parameters>(len);
  m_integralPion = std::vector<double>(len);

  m_muonlike.m_params = std::vector<ECLMuonPdf::Parameters>(len);
  m_muonlike.m_integral1 = std::vector<double>(len);
  m_muonlike.m_integral2 = std::vector<double>(len);

  ECLAbsPdf& abs = m_muonlike;
  abs.init(map);

  unsigned int i(len - 1);

  for (unsigned int ip(0); ip < n_p_bins; ++ip) {

    for (unsigned int ith(0); ith < n_theta_bins; ++ith) {

      i = index(ip, ith);

      //Build muon-like PDF (Bifurcated Gaussian+Gaussian)
      ECLMuonPdf::Parameters& prm = m_muonlike.m_params[i];
      prm.mu1 = map.param(name("pions_mu1_", ip, ith));
      prm.mu2 = map.param(name("pions_mu2_", ip, ith));
      prm.sigma1l = map.param(name("pions_sigma1l_", ip, ith));
      prm.sigma1r = map.param(name("pions_sigma1r_", ip, ith));
      prm.sigma2 = map.param(name("pions_sigma2_", ip, ith));
      prm.fraction = map.param(name("pions_fraction1_", ip, ith));

      m_muonlike.m_integral1[i] = 0.5 * (1 - TMath::Erf(- prm.mu1  / prm.sigma1l / s_sqrt2));
      m_muonlike.m_integral2[i] = 0.5 * (1 - TMath::Erf(- prm.mu2  / prm.sigma2 / s_sqrt2));

      // Build Gaussian PDF
      Parameters& pi_prm = m_params[i];
      pi_prm.mu3 = map.param(name("pions_mu3_", ip, ith));
      pi_prm.sigma3 = map.param(name("pions_sigma3_", ip, ith));
      pi_prm.fraction = map.param(name("pions_fraction2_", ip, ith));

      m_integralPion[i] = 0.5 * (1 - TMath::Erf(-pi_prm.mu3 / pi_prm.sigma3 / s_sqrt2));

    }
  }
}

double ECLPionPdf::pdf(const double& eop, const double& p, const double& theta) const
{

  unsigned int i = index(p, theta);

  const Parameters& prm = m_params[i];

  return prm.fraction * m_muonlike.pdf(eop , p, costheta) +
         (1 - prm.fraction) * TMath::Gaus(eop, prm.mu3, prm.sigma3, true) / m_integralPion[i];
}
