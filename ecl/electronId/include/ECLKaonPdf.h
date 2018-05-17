/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Class for ECL E/p kaon PDF                                             *
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
#include <ecl/electronId/ECLMuonPdf.h>

namespace Belle2 {

  namespace ECL {

    /** Class for ECL E/p kaon PDF
     * Components:
     *   1. Bifurcated gaussian (as for muons)
     *   2. Gaussian (as for muons)
     *   3. Gaussian
     */
    class ECLKaonPdf : public ECLAbsPdf {
    public:
      double pdf(const double& eop, const double& p, const double& theta) const;
      void init(const char* parametersFileName);

      struct Parameters {
        double fraction;
        double mu3;
        double sigma3;
      };

      Parameters* pdfParams(const double& p, const double& theta)
      {
        return &m_params[index(p, theta)];
      };

      ECLMuonPdf::Parameters* pdfParamsMu(const double& p, const double& theta)
      {
        return m_muonlike.pdfParams(p, theta);
      };

    private:

      ECLMuonPdf m_muonlike;
      std::vector<Parameters> m_params;
      std::vector<double> m_integralKaon;

    };

  }
}
