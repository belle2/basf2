#ifndef ECLPIONPDF_H
#define ECLPIONPDF_H

#include <ecl/electronId/ECLAbsPdf.h>
#include <ecl/electronId/ECLMuonPdf.h>

namespace Belle2 {
  class ECLPionPdf : public ECLAbsPdf {
  public:
    double pdf(double eop, double p, double costheta) const;
    void init(const char* parametersFileName);
    ~ECLPionPdf();
    struct Parameters {
      double fraction;
      double mu3;
      double sigma3;
    };
    Parameters* m_params;
    ECLMuonPdf m_muonlike;
    double* m_integralPion;
  };
}
#endif
