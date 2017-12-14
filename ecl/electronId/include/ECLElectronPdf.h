#pragma once

#include <vector>

#include <ecl/electronId/ECLAbsPdf.h>

namespace Belle2 {
  namespace ECL {
    class ECLElectronPdf : public ECLAbsPdf {
    public:
      double pdf(double eop, double p, double costheta) const;
      void init(const char* parametersFileName);

    private:

      struct Parameters {
        double mu1;
        double sigma1;
        double mu2;
        double sigma2;
        double fraction;
        double alpha;
        double nn;
      };

      std::vector<Parameters> m_params;
      std::vector<double> m_integral1;
      std::vector<double> m_integral2;

    };
  }
}
