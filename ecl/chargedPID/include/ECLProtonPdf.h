/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Class for ECL E/p proton PDF                                           *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

#include <ecl/chargedPID/ECLAbsPdf.h>
#include <ecl/chargedPID/ECLMuonPdf.h>

namespace Belle2 {

  namespace ECL {

    /** Class for ECL E/p proton PDF.
     * Components:
     *   1. Bifurcated gaussian (as for muons)
     *   2. Gaussian (as for muons)
     *   3. Gaussian
     Please see docs of ECLPionPDF class for details.
     */
    class ECLProtonPdf : public ECLAbsPdf {
    public:

      void init(const char* parametersFileName);

      struct Parameters {
        double mu3;
        double sigma3;
        double fraction;
      };

      Parameters* pdfParams(const double& p, const double& theta)
      {
        return &m_params[index(p, theta)];
      };

      Parameters* pdfParams(unsigned int idx_p, unsigned int idx_th)
      {
        return &m_params[index(idx_p, idx_th)];
      };

      ECLMuonPdf::Parameters* pdfParamsMu(const double& p, const double& theta)
      {
        return m_muonlike.pdfParams(p, theta);
      };

      ECLMuonPdf::Parameters* pdfParamsMu(unsigned int idx_p, unsigned int idx_th)
      {
        return m_muonlike.pdfParams(idx_p, idx_th);
      };

    private:

      /** The value of the pdf
       */
      double pdffunc(const double& eop, unsigned int i) const;

      ECLMuonPdf m_muonlike;
      std::vector<Parameters> m_params;
      std::vector<double> m_integralProton;

    };

  }
}
