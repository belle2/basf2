#pragma once

#include <vector>

#include <ecl/electronId/ECLAbsPdf.h>
#include <ecl/electronId/ECLMuonPdf.h>

namespace Belle2 {

  namespace ECL {

    class ECLPionPdf : public ECLAbsPdf {
    public:
      double pdf(const double& eop, const double& p, const double& theta) const;
      void init(const char* parametersFileName);

      struct Parameters {
        double fraction;
        double mu3;
        double sigma3;
      };

    private:

      ECLMuonPdf m_muonlike;
      std::vector<Parameters> m_params;
      std::vector<double> m_integralPion;

    };

  }
}
