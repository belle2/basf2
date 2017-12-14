#pragma once

#include <vector>

#include <ecl/electronId/ECLAbsPdf.h>


namespace Belle2 {
  namespace ECL {
    class ECLMuonPdf : public ECLAbsPdf {
    public:
      double pdf(double eop, double p, double costheta) const;
      void init(const char* parametersFileName) ;

      struct Parameters {
        double mu1;
        double sigma1l;
        double sigma1r;
        double mu2;
        double sigma2;
        double fraction;
      };

      std::vector<Parameters> m_params;
      std::vector<double> m_integral1;
      std::vector<double> m_integral2;

    };
  }
}
