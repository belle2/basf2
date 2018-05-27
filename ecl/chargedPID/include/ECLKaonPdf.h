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

#include <ecl/chargedPID/ECLAbsPdf.h>
#include <ecl/chargedPID/ECLMuonPdf.h>

namespace Belle2 {

  namespace ECL {

    /** Class for ECL E/p kaon PDF.
     * Components:
     *   1. Bifurcated gaussian (as for muons)
     *   2. Gaussian (as for muons)
     *   3. Gaussian
     */
    class ECLKaonPdf : public ECLAbsPdf {
    public:

      /** Build the E/p PDF for kaons from the input fitted parameters.
      The normalisation factor is calculated between E/p = 0 and +infty:

      1. Bifurcated Gaussian:
      (http://mathworld.wolfram.com/NormalDistribution.html).

      2. Gaussian:
      (http://mathworld.wolfram.com/NormalDistribution.html).

      3. Gaussian:
      */
      void init(const char* parametersFileName);

      /** @struct Parameters
      @brief Parameters of the E/p ECL PDF for kaons.
      These include only parameters of the PDFs that are not included in the muon case.
      @var Parameters::mu3 mean of Gaussian.
      @var Parameters::sigma3 std dev of Gaussian.
      @var Parameters::fraction Gaussian fraction (see RooAddPdf docs).
       */
      struct Parameters {
        double mu3; /**< mean of Gaussian. */
        double sigma3; /**< std dev of Gaussian. */
        double fraction; /**< Gaussian fraction (see RooAddPdf docs). */
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

      /** Pointer to the struct containing the PDf parameters of the muon-like components:
      @param p the particle's momentum.
      @param theta the particle's theta.
       */
      ECLMuonPdf::Parameters* pdfParamsMu(const double& p, const double& theta)
      {
        return m_muonlike.pdfParams(p, theta);
      };

      /** Pointer to the struct containing the PDf parameters of the muon-like components:
      @param idx_p the particle's momentum bin index.
      @param idx_th the particle's theta bin index.
       */
      ECLMuonPdf::Parameters* pdfParamsMu(unsigned int idx_p, unsigned int idx_th)
      {
        return m_muonlike.pdfParams(idx_p, idx_th);
      };

    private:

      /** Calculate the value of the PDF:
      @param eop the particle's E/p.
      @param i the global (p,theta) bin index of the particle.
       */
      double pdffunc(const double& eop, unsigned int i) const;

      /** Muon PDF class object to store muon-like PDF parameters
       */
      ECLMuonPdf m_muonlike;

      /** List of all PDF parameters for each (p,theta) bin.
       */
      std::vector<Parameters> m_params;
      /** Normalisation factor of Gaussian component.
       */
      std::vector<double> m_integralKaon;

    };

  }
}
