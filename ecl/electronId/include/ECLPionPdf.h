#ifndef ECLPIONPDF_H
#define ECLPIONPDF_H

#include <ecl/electronId/ECLAbsPdf.h>
#include <ecl/electronId/ECLMuonPdf.h>

namespace Belle2 {
  class ECLPionPdf : public ECLAbsPdf {
  public:
    double pdf(double eop, double p) const;
    void init();

    struct Parameters {
      double fraction;
      double mu3;
      double sigma3;
    };
    Parameters m_params[8];
    ECLMuonPdf m_muonlike;
  };
}
#endif
