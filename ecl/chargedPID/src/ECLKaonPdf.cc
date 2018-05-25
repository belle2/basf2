#include <ecl/chargedPID/ECLKaonPdf.h>
#include <ecl/chargedPID/ParameterMap.h>
#include <cmath>
#include <TMath.h>

using namespace Belle2;
using namespace ECL;

void ECLKaonPdf::init(const char* parametersFileName)
{

  bool isAntiPart = (std::string(parametersFileName).find("anti") != std::string::npos);
  std::string chargePrefix = (!isAntiPart) ? "" : "anti";

  ParameterMap map(parametersFileName);
  ECLAbsPdf::init(map);

  unsigned int len = n_p_bins * n_theta_bins;

  m_params = std::vector<Parameters>(len);
  m_integralKaon = std::vector<double>(len);

  m_muonlike.m_params = std::vector<ECLMuonPdf::Parameters>(len);
  m_muonlike.m_integral1 = std::vector<double>(len);
  m_muonlike.m_integral2 = std::vector<double>(len);

  ECLAbsPdf& abs = m_muonlike;
  abs.init(map);

  unsigned int i(len - 1);

  for (unsigned int ip(0); ip < n_p_bins; ++ip) {

    for (unsigned int ith(0); ith < n_theta_bins; ++ith) {

      i = index(ip, ith);

      // Build muon-like PDF (Bifurcated Gaussian+Gaussian)
      ECLMuonPdf::Parameters& prm = m_muonlike.m_params[i];
      prm.mu1 = map.param(name((chargePrefix + "kaons_mu1_").c_str(), ip, ith));
      prm.mu2 = map.param(name((chargePrefix + "kaons_mu2_").c_str(), ip, ith));
      prm.sigma1l = map.param(name((chargePrefix + "kaons_sigma1l_").c_str(), ip, ith));
      prm.sigma1r = map.param(name((chargePrefix + "kaons_sigma1r_").c_str(), ip, ith));
      prm.sigma2 = map.param(name((chargePrefix + "kaons_sigma2_").c_str(), ip, ith));
      prm.fraction = map.param(name((chargePrefix + "kaons_fraction1_").c_str(), ip, ith));

      m_muonlike.m_integral1[i] = 0.5 * (1 - TMath::Erf(- prm.mu1  / prm.sigma1l / s_sqrt2));
      m_muonlike.m_integral2[i] = 0.5 * (1 - TMath::Erf(- prm.mu2  / prm.sigma2 / s_sqrt2));

      // Build Gaussian PDF
      Parameters& pi_prm = m_params[i];
      pi_prm.mu3 = map.param(name((chargePrefix + "kaons_mu3_").c_str(), ip, ith));
      pi_prm.sigma3 = map.param(name((chargePrefix + "kaons_sigma3_").c_str(), ip, ith));
      pi_prm.fraction = map.param(name((chargePrefix + "kaons_fraction2_").c_str(), ip, ith));

      m_integralKaon[i] = 0.5 * (1 - TMath::Erf(-pi_prm.mu3 / pi_prm.sigma3 / s_sqrt2));

    }
  }
}

double ECLKaonPdf::pdffunc(const double& eop, unsigned int i) const
{

  const Parameters& prm = m_params[i];

  return prm.fraction * m_muonlike.pdffunc(eop, i) +
         (1 - prm.fraction) * TMath::Gaus(eop, prm.mu3, prm.sigma3, true) / m_integralKaon[i];
}
