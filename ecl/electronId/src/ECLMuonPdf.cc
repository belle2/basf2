#include <ecl/electronId/ECLMuonPdf.h>
#include <ecl/electronId/ParameterMap.h>

#include <cmath>
#include <TMath.h>

using namespace Belle2;
using namespace ECL;

void  ECLMuonPdf::init(const char* parametersFileName)
{
  ParameterMap map(parametersFileName);
  ECLAbsPdf::init(map);

  unsigned int len = nbins * nCosTheta ;
  m_params = std::vector<Parameters>(len);
  m_integral1 = std::vector<double>(len);
  m_integral2 = std::vector<double>(len);
  for (unsigned int ip = 1; ip < nbins; ++ip)
    for (unsigned int ith = 0; ith < nCosTheta; ++ith) {
      unsigned int i = index(ip, ith);
      Parameters& prm = m_params[i];
      prm.mu1 = map.param(name("muons_mu1_", ip, ith));
      prm.mu2 = map.param(name("muons_mu2_", ip, ith));
      prm.sigma1l = map.param(name("muons_sigma1l_", ip, ith));
      prm.sigma1r = map.param(name("muons_sigma1r_", ip, ith));
      prm.sigma2 = map.param(name("muons_sigma2_", ip, ith));
      prm.fraction = map.param(name("muons_fraction_", ip, ith));

      m_integral1[i] = 0.5 * (1 - TMath::Erf(- prm.mu1  / prm.sigma1l / s_sqrt2));
      m_integral2[i] = 0.5 * (1 - TMath::Erf(- prm.mu2  / prm.sigma2 / s_sqrt2));
    }
}

double ECLMuonPdf::pdf(double eop, double p, double costheta) const
{
  unsigned int i = index(p, costheta);
  const Parameters& prm = m_params[i];
  double sigma1 = (eop < prm.mu1) ? prm.sigma1l : prm.sigma1r;
  return TMath::Gaus(eop, prm.mu1, sigma1, true) * prm.fraction / m_integral1[i] +
         (1 - prm.fraction) * TMath::Gaus(eop, prm.mu2, prm.sigma2, true) / m_integral2[2];
}
