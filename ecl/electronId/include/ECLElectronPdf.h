#ifndef ECLELECTRONPDF_H
#define ECLELECTRONPDF_H

#include <ecl/electronId/ECLAbsPdf.h>

namespace Belle2 {
  class ECLElectronPdf : public ECLAbsPdf {
  public:
    double pdf(double eop, double p) const;
    void init();
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

    Parameters m_params[8];
    double m_integral1[8];
    double m_integral2[8];

  };
}
#endif
