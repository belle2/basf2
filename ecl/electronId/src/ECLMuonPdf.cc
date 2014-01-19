#include <ecl/electronId/ECLMuonPdf.h>
#include <ecl/electronId/ParameterMap.h>

#include <cmath>
#include <TMath.h>

using namespace Belle2;

void  ECLMuonPdf::init(const char* parametersFileName)
{
  ParameterMap map(parametersFileName);
  for (int i = 1; i < 8; ++i) {
    Parameters& prm = m_params[i];
    prm.mu1 = map.param(name("muons_mu1_", i));
    prm.mu2 = map.param(name("muons_mu2_", i));
    prm.sigma1l = map.param(name("muons_sigma1l_", i));
    prm.sigma1r = map.param(name("muons_sigma1r_", i));
    prm.sigma2 = map.param(name("muons_sigma2_", i));
    prm.fraction = map.param(name("muons_fraction_", i));

    m_integral1[i] = 0.5 * (1 - TMath::Erf(- prm.mu1  / prm.sigma1l / s_sqrt2));
    m_integral2[i] = 0.5 * (1 - TMath::Erf(- prm.mu2  / prm.sigma2 / s_sqrt2));
  }

}

double ECLMuonPdf::pdf(double eop, double p) const
{
  int i = int(p / 0.250);
  if (i == 0) i = 1;
  if (i > 7) i = 7;
  const Parameters& prm = m_params[i];
  double sigma1 = (eop < prm.mu1) ? prm.sigma1l : prm.sigma1r;
  return TMath::Gaus(eop, prm.mu1, sigma1, true) * prm.fraction / m_integral1[i] +
         (1 - prm.fraction) * TMath::Gaus(eop, prm.mu2, prm.sigma2, true) / m_integral2[2];

}


