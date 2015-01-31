#ifndef ECLMUONPDF_H
#define ECLMUONPDF_H

#include <ecl/electronId/ECLAbsPdf.h>

namespace Belle2 {
  class ECLMuonPdf : public ECLAbsPdf {
  public:
    double pdf(double eop, double p, double costheta) const;
    void init(const char* parametersFileName) ;
    ~ECLMuonPdf();
    struct Parameters {
      double mu1;
      double sigma1l;
      double sigma1r;
      double mu2;
      double sigma2;
      double fraction;
    };

    Parameters* m_params;
    double* m_integral1;
    double* m_integral2;
  };
}
#endif
