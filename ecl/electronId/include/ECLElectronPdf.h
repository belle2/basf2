/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Class for ECL E/p electron PDF                                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

#include <ecl/electronId/ECLAbsPdf.h>

namespace Belle2 {

  namespace ECL {

    /** Class for ECL E/p electron PDF
     * Components:
     *   1. Gaussian
     *   2. Crystal Ball
     */
    class ECLElectronPdf : public ECLAbsPdf {
    public:

      double pdf(const double& eop, const double& p, const double& theta) const;
      void init(const char* parametersFileName);

      struct Parameters {
        double mu1;
        double sigma1;
        double mu2;
        double sigma2;
        double fraction;
        double alpha;
        double nn;
      };

      Parameters* pdfParams(const double& p, const double& theta)
      {
        return &m_params[index(p, theta)];
      };

    private:

      std::vector<Parameters> m_params;
      std::vector<double> m_integral1;
      std::vector<double> m_integral2;

    };
  }
}
