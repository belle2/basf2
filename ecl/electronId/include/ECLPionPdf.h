#pragma once
#include <vector>

#include <ecl/electronId/ECLAbsPdf.h>
#include <ecl/electronId/ECLMuonPdf.h>

namespace Belle2 {
  namespace ECL {
    class ECLPionPdf : public ECLAbsPdf {
    public:
      double pdf(double eop, double p, double costheta) const;
      void init(const char* parametersFileName);

    private:
      struct Parameters {
        double fraction;
        double mu3;
        double sigma3;
      };

      ECLMuonPdf m_muonlike;
      std::vector<Parameters> m_params;
      std::vector<double> m_integralPion;
    };
  }
}
