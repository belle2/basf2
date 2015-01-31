#ifndef ECLELECTRONPDF_H
#define ECLELECTRONPDF_H

#include <ecl/electronId/ECLAbsPdf.h>

namespace Belle2 {
  class ECLElectronPdf : public ECLAbsPdf {
  public:
    double pdf(double eop, double p, double costheta) const;
    void init(const char* parametersFileName);
    ~ECLElectronPdf();
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

    Parameters* m_params;
    double* m_integral1;
    double* m_integral2;

  };
}
#endif
