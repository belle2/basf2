/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Class for ECL E/p muon PDF                                             *
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

namespace Belle2 {

  namespace ECL {

    /** Class for ECL E/p muon PDF.
     * Components:
     *   1. Bifurcated gaussian
     *   2. Gaussian
     */
    class ECLMuonPdf : public ECLAbsPdf {
      friend class ECLPionPdf;
      friend class ECLProtonPdf;
      friend class ECLKaonPdf;
    public:

      /** Build the E/p PDF for muons from the input fitted parameters.
      */
      void init(const char* parametersFileName);

      /** @struct Parameters
      @brief Parameters of the E/p ECL PDF for muons.
       */
      struct Parameters {
        double mu1; /**< mean of Bifurcated Gaussian. */
        double sigma1l; /**< std dev (left side) of Bifurcated Gaussian. */
        double sigma1r; /**< std dev (right side) of Bifurcated Gaussian. */
        double fraction; /**< Bifurcated Gaussian fraction (see RooAddPdf docs). */
        double mu2; /**< mean of Gaussian. */
        double sigma2; /**< std dev of Gaussian.*/
        double fitrange_dn; /**< The lower bound of the E/p range where the PDF was fitted. */
        double fitrange_up; /**< The upper bound of the E/p range where the PDF was fitted. */
      };

      /** Pointer to the struct containing the PDf parameters:
      @param p the particle's momentum.
      @param theta the particle's theta.
       */
      Parameters* pdfParams(const double& p, const double& theta)
      {
        return &m_params[index(p, theta)];
      };

      /** Pointer to the struct containing the PDf parameters:
      @param idx_p the particle's momentum bin index.
      @param idx_th the particle's theta bin index.
       */
      Parameters* pdfParams(unsigned int idx_p, unsigned int idx_th)
      {
        return &m_params[index(idx_p, idx_th)];
      };

    private:

      /** List of all PDF parameters for each (p,theta) bin.
       */
      std::vector<Parameters> m_params;

    };
  }
}
