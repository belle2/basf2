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

#include <ecl/chargedPID/ECLAbsPdf.h>

namespace Belle2 {

  namespace ECL {

    /** Class for ECL E/p electron PDF.
     * Components:
     *   1. Gaussian.
     *   2. Crystal Ball.
     */
    class ECLElectronPdf : public ECLAbsPdf {
    public:

      /** Build the E/p PDF for electrons from the input fitted parameters.
      The normalisation factor is calculated between E/p = 0 and +infty:

      1. Gaussian:
      (http://mathworld.wolfram.com/NormalDistribution.html).

      G(E/p) = 1/sqrt(2*pi)*sigma1 * exp( -(E/p-mu1)^2 / 2*sigma^2 )

      N = int_{0}^{+infty} G(E/p)
      = int_{-infty}^{+infty} G(E/p) - int_{-infty}^{0} G(E/p)
      = 1 - 1/2 * [ 1 + erf( (0 -mu1) / sqrt(2) * sigma1 ) ]

      2. CB:
      (https://en.wikipedia.org/wiki/Crystal_Ball_function).
      */
      void init(const char* parametersFileName);

      /** @struct Parameters
      @brief Parameters of the E/p ECL PDF for electrons:
      @var Parameters::mu1 mean of Gaussian.
      @var Parameters::sigma1 std dev of Gaussian.
      @var Parameters::fraction Gaussian fraction (see RooAddPdf docs).
      @var Parameters::mu2 mean of CB Gaussian core.
      @var Parameters::sigma2 std dev of CB Gaussian core.
      @var Parameters::alpha alpha paremeter of CB.
      @var Parameters::nn n parameter of CB.
       */
      struct Parameters {
        double mu1;
        double sigma1;
        double fraction;
        double mu2;
        double sigma2;
        double alpha;
        double nn;
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

      /** Calculate the value of the PDF:
      @param eop the particle's E/p.
      @param i the global (p,theta) bin index of the particle.
       */
      double pdffunc(const double& eop, unsigned int i) const;

      /** List of all PDF parameters for each (p,theta) bin.
       */
      std::vector<Parameters> m_params;
      /** Normalisation factor of Gaussian component.
       */
      std::vector<double> m_integral1;
      /** Normalisation factor of CB component.
       */
      std::vector<double> m_integral2;

    };
  }
}
