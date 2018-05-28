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

// ROOT/RooFit includes
#include <RooRealVar.h>
#include <RooAddPdf.h>

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
      */
      void init(const char* parametersFileName);

      /** @struct Parameters
      @brief Parameters of the E/p ECL PDF for electrons.
       */
      struct Parameters {
        double mu1; /**< mean of Gaussian. */
        double sigma1; /**< std dev of Gaussian. */
        double fraction; /**< Gaussian fraction (see RooAddPdf docs). */
        double mu2; /**< mean of CB Gaussian core. */
        double sigma2; /**< std dev of CB Gaussian core. */
        double alpha; /**< alpha paremeter of CB. */
        double nn; /**< n parameter of CB. */
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

      /** Calculate the value of the PDF:
      @param eop the particle's E/p.
      @param i the global (p,theta) bin index of the particle.
       */
      double pdffunc(const double& eop, unsigned int i) const;

      /** List of all PDF parameters for each (p,theta) bin.
       */
      std::vector<Parameters> m_params;

      /** List of the RooRealVar observables for each (p,theta) bin.
      Each bin can have a different range for the observable.
       */
      std::vector<RooRealVar> m_vars;

      /** List of the RooFit PDFs for each (p,theta) bin.
       */
      std::vector<RooAddPdf> m_PDFs;

    };
  }
}
