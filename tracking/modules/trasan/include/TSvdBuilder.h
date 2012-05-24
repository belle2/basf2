#ifndef _DEFINE_TSVD_BUILDER_H_
#define _DEFINE_TSVD_BUILDER_H_

#define HEP_SHORT_NAMES

#include "tracking/modules/trasan/AList.h"

namespace Belle {

  class TSvdHit;
  class TSvdDssd;

  class TSvdBuilder {
  public:
    // Constructor
    TSvdBuilder(void) {};

    // Destructor
    virtual ~TSvdBuilder(void) {};

    // Calculator
    unsigned calTHelix3(double x, double y, double r, double q,
                        double& dRho, double& phi0, double& kappa);

    unsigned calZTanL(double dRho, double phi0, double kappa,
                      double& dZ, double& tanL, double& chisq,
                      AList<TSvdHit> hits,
                      double ipC = 0.);

    unsigned calTHelix(double x, double y, double r, double q,
                       double& dRho, double& phi0, double& kappa,
                       double& dZ, double& tanL, double& chisq,
                       AList<TSvdHit> hits,
                       double ipC = 0.);
  private:
  };

} // namespace Belle

#endif /* _DEFINE_TSVD_BUIIDER_H_ */
