#pragma once

#include <vector>

#include <ecl/electronId/ECLAbsPdf.h>

namespace Belle2 {

  namespace ECL {

    class ECLMuonPdf : public ECLAbsPdf {
      friend class ECLPionPdf;
    public:

      double pdf(const double& eop, const double& p, const double& theta) const;
      void init(const char* parametersFileName);

      struct Parameters {
        double mu1;
        double sigma1l;
        double sigma1r;
        double mu2;
        double sigma2;
        double fraction;
      };

    private:

      std::vector<Parameters> m_params;
      std::vector<double> m_integral1;
      std::vector<double> m_integral2;

    };
  }
}
