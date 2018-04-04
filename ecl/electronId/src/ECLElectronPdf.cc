#include <ecl/electronId/ECLElectronPdf.h>
#include <ecl/electronId/ParameterMap.h>

#include <cmath>
#include <TMath.h>

using namespace Belle2;
using namespace ECL;

void ECLElectronPdf::init(const char* parametersFileName)
{

  bool isAntiPart = (std::string(parametersFileName).find("anti") != std::string::npos);
  std::string chargePrefix = (!isAntiPart) ? "" : "anti";

  ParameterMap map(parametersFileName);
  ECLAbsPdf::init(map);

  unsigned int len = n_p_bins * n_theta_bins;

  m_params    = std::vector<Parameters>(len);
  m_integral1 = std::vector<double>(len);
  m_integral2 = std::vector<double>(len);

  unsigned int i(len - 1);

  for (unsigned int ip(0); ip < n_p_bins; ++ip) {

    for (unsigned int ith(0); ith < n_theta_bins; ++ith) {

      i = index(ip, ith); // global index in linearised (p,theta) matrix

      Parameters& prm = m_params[i];

      // Build Gaussian PDF
      prm.mu1 = map.param(name((chargePrefix + "electrons_mu1_").c_str(), ip, ith));
      prm.sigma1 = map.param(name((chargePrefix + "electrons_sigma1_").c_str(), ip, ith));
      prm.fraction = map.param(name((chargePrefix + "electrons_fraction_").c_str(), ip, ith)); //Gaussian fraction coefficient

      m_integral1[i] = 0.5 * (1 - TMath::Erf(- prm.mu1 / prm.sigma1 / s_sqrt2));

      // Build Crystal ball PDF
      double m0       = prm.mu2 = map.param(name((chargePrefix + "electrons_mu2_").c_str(), ip, ith));
      double sigma    = prm.sigma2 = map.param(name((chargePrefix + "electrons_sigma2_").c_str(), ip, ith));
      double absAlpha = TMath::Abs(prm.alpha = map.param(name((chargePrefix + "electrons_alpha_").c_str(), ip, ith)));
      double n        = prm.nn = map.param(name((chargePrefix + "electrons_nn_").c_str(), ip, ith));

      double sig = TMath::Abs(sigma);
      double tmin = - m0 / sig;
      if (tmin >= -absAlpha) {
        m_integral2[i] = 0.5 * (1 - TMath::Erf(- prm.mu2 / prm.sigma2 / s_sqrt2));
      } else {
        double a = TMath::Power(n / absAlpha, n) * std::exp(-0.5 * absAlpha * absAlpha);
        double b = n / absAlpha - absAlpha;
        double term1 = a * sig / (1.0 - n) * (1.0 / (TMath::Power(b - tmin, n - 1.0)) - 1.0 / (TMath::Power(n / absAlpha, n - 1.0)));
        double term2 = sig * s_sqrtPiOver2 * (1 - TMath::Erf(-absAlpha / s_sqrt2));
        m_integral2[i] = (term1 + term2);
      }

    }

  }

}

double ECLElectronPdf::pdf(const double& eop, const double& p, const double& theta) const
{

  unsigned int i = index(p, theta);

  const Parameters& prm = m_params[i];

  double t1 = TMath::Gaus(eop, prm.mu1, prm.sigma1, true) / m_integral1[i];

  double t2;
  double m0 = prm.mu2;
  double sigma = prm.sigma2;
  double alpha = prm.alpha;
  double n = prm.nn;
  double t = (eop - m0) / sigma;
  Double_t absAlpha = TMath::Abs(alpha);

  if (t >= -absAlpha) {
    t2 = std::exp(-0.5 * t * t) ;
  } else {
    Double_t a =  TMath::Power(n / absAlpha, n) * std::exp(-0.5 * absAlpha * absAlpha);
    Double_t b = n / absAlpha - absAlpha;
    t2 = a / TMath::Power(b - t, n);
  }
  t2 /= m_integral2[i];

  return prm.fraction * t1 + (1 - prm.fraction) * t2;

}
