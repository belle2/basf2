#include <ecl/electronId/ECLPionPdf.h>
#include <ecl/electronId/ParameterMap.h>
#include <cmath>
#include <TMath.h>

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
  }

}

double ECLPionPdf::pdf(double eop, double p) const
{
  int i = int(p / 0.250);
  if (i == 0) return 1e-20;
  if (i > 7) i = 7;
  const Parameters& prm = m_params[i];
  return prm.fraction * m_muonlike.pdf(eop , p) +
         (1 - prm.fraction) * TMath::Gaus(eop, prm.mu3, prm.sigma3, true);
}
