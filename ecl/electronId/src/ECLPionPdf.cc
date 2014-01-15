#include <ecl/electronId/ECLPionPdf.h>
#include <ecl/electronId/ParameterMap.h>
#include <cmath>
#include <TMath.h>

using namespace std;
using namespace Belle2;

void  ECLPionPdf::init()
{

  ParameterMap map("ecl/data/pions.dat");

  for (int i = 1; i < 8; ++i) {
    ECLMuonPdf::Parameters& prm = m_muonlike.m_params[i];
    prm.mu1 = map.param(name("pions_mu1_", i));
    prm.mu2 = map.param(name("pions_mu2_", i));
    prm.sigma1l = map.param(name("pions_sigma1l_", i));
    prm.sigma1r = map.param(name("pions_sigma1r_", i));
    prm.sigma2 = map.param(name("pions_sigma2_", i));
    prm.fraction = map.param(name("pions_fraction1_", i));
    Parameters& pi_prm = m_params[i];
    pi_prm.mu3 = map.param(name("pions_mu3_", i));
    pi_prm.sigma3 = map.param(name("pions_sigma3_", i));
    pi_prm.fraction = map.param(name("pions_fraction2_", i));

    m_muonlike.m_integral1[i] = 0.5 * (1 - TMath::Erf(- prm.mu1  / prm.sigma1l / s_sqrt2));
    m_muonlike.m_integral2[i] = 0.5 * (1 - TMath::Erf(- prm.mu2  / prm.sigma2 / s_sqrt2));
    m_integralPion[i] = 0.5 * (1 - TMath::Erf(-pi_prm.mu3 / pi_prm.sigma3 / s_sqrt2));
    cerr << "integral pion " << m_integralPion[i] << endl;
  }
}

double ECLPionPdf::pdf(double eop, double p) const
{
  int i = int(p / 0.250);
  if (i == 0) i = 1;
  if (i > 7) i = 7;
  const Parameters& prm = m_params[i];
  return prm.fraction * m_muonlike.pdf(eop , p) +
         (1 - prm.fraction) * TMath::Gaus(eop, prm.mu3, prm.sigma3, true) / m_integralPion[i];
}
