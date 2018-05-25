#include <ecl/chargedPID/ECLMuonPdf.h>
#include <ecl/chargedPID/ParameterMap.h>

#include <cmath>
#include <TMath.h>

using namespace Belle2;
using namespace ECL;

void ECLMuonPdf::init(const char* parametersFileName)
{

  bool isAntiPart = (std::string(parametersFileName).find("anti") != std::string::npos);
  std::string chargePrefix = (!isAntiPart) ? "" : "anti";

  ParameterMap map(parametersFileName);
  ECLAbsPdf::init(map);

  unsigned int len = n_p_bins * n_theta_bins;

  m_params = std::vector<Parameters>(len);
  m_integral1 = std::vector<double>(len);
  m_integral2 = std::vector<double>(len);

  unsigned int i(len - 1);

  for (unsigned int ip(0); ip < n_p_bins; ++ip) {

    for (unsigned int ith(0); ith < n_theta_bins; ++ith) {

      i = index(ip, ith);

      Parameters& prm = m_params[i];

      // Build Bifurcated Gaussian PDF
      prm.mu1 = map.param(name((chargePrefix + "muons_mu1_").c_str(), ip, ith));
      prm.sigma1l = map.param(name((chargePrefix + "muons_sigma1l_").c_str(), ip, ith));
      prm.sigma1r = map.param(name((chargePrefix + "muons_sigma1r_").c_str(), ip, ith));
      prm.fraction = map.param(name((chargePrefix + "muons_fraction_").c_str(), ip, ith));

      m_integral1[i] = 0.5 * (1 - TMath::Erf(- prm.mu1  / prm.sigma1l / s_sqrt2));

      // Build Gaussian PDF
      prm.mu2 = map.param(name((chargePrefix + "muons_mu2_").c_str(), ip, ith));
      prm.sigma2 = map.param(name((chargePrefix + "muons_sigma2_").c_str(), ip, ith));

      m_integral2[i] = 0.5 * (1 - TMath::Erf(- prm.mu2  / prm.sigma2 / s_sqrt2));

    }

  }

}

double ECLMuonPdf::pdffunc(const double& eop, unsigned int i) const
{

  const Parameters& prm = m_params[i];

  double sigma1 = (eop < prm.mu1) ? prm.sigma1l : prm.sigma1r;
  double bifurgaus1 = TMath::Gaus(eop, prm.mu1, sigma1, true) / m_integral1[i];

  double gaus2 = TMath::Gaus(eop, prm.mu2, prm.sigma2, true) / m_integral2[2];

  return prm.fraction * bifurgaus1 + (1 - prm.fraction) * gaus2;

}
