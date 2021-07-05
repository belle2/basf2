/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <TH1F.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Algorithm for module T0 calibration with chi2 minimization of time differences
     * between slots (method DeltaT).
     *
     * For the method see M. Staric, NIM A 586 (2008) 174-179, sect. 2.2.
     *
     * Useful for initial (rough) calibration, since the results are found slightly biased
     * For the final (precise) calibration one has to use LL method.
     */
    class TOPModuleT0DeltaTAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor */
      TOPModuleT0DeltaTAlgorithm();

      /** Destructor */
      virtual ~TOPModuleT0DeltaTAlgorithm() {}

      /**
       * Sets minimal number of histogram entries to perform a fit
       */
      void setMinEntries(int minEntries) {m_minEntries = minEntries;}

      /**
       * Sets values for the initialization of several fit parameters
       * @param sigmaCore core gaussian sigma [ns]
       * @param sigmaTail tail gaussian sigma [ns]
       * @param tailFract fraction of tail gaussian. Single gaussian used if set to 0.
       */
      void setFitInitializers(double sigmaCore, double sigmaTail, double tailFract)
      {
        m_sigmaCoreInit = sigmaCore;
        m_sigmaTailInit = sigmaTail;
        m_tailFractInit = tailFract;
      }

      /**
       * Sets cutoff on the number of histogram entries for steering btw. single/double
       * gaussian parameterization of the signal.
       * @param cutoffEntries double gaussian above that value, otherwise single gaussian
       */
      void setCutoffEntries(int cutoffEntries) {m_cutoffEntries = cutoffEntries;}

      /**
       * Sets minimal result uncertainty to declare this calibration as c_OK
       * @param minError minimal uncertainty [ns]
       */
      void setMinError(double minError) {m_minError = minError;}

    private:

      /**
       * algorithm implementation
       */
      virtual EResult calibrate() final;

      /**
       * Fit single gaus + constant
       * @param h histogram
       * @return fit status
       */
      int fitSingleGaus(std::shared_ptr<TH1F> h);

      /**
       * Fit double gaus w/ same mean + constant
       * @param h histogram
       * @return fit status
       */
      int fitDoubleGaus(std::shared_ptr<TH1F> h);

      /**
       * Fit histogram
       * @param h histogram
       * @return fit status
       */
      int fitHistogram(std::shared_ptr<TH1F> h);

      // algorithm parameters
      int m_minEntries = 10; /**< minimal number of histogram entries to perform fit */
      double m_sigmaCoreInit = 0.120; /**< core gaussian sigma [ns] */
      double m_sigmaTailInit = 0.240; /**< tail gaussian sigma [ns] */
      double m_tailFractInit = 0.20; /**< fraction of tail gaussian */
      int m_cutoffEntries = 100; /**< cutoff entries for single/double gaussian fit */
      double m_minError = 0.100; /**< minimal moduleT0 uncertainty [ns] to declare c_OK */

      // temporary store for results of a single fit
      double m_delT0 = 0; /**< fitted delta T0 */
      double m_error = 0; /**< error on fitted delta T0 */
      double m_chi2 = 0;  /**< chi2 of the fit */
      double m_ndf = 0;  /**< NDF of the fit */

    };

  } // end namespace TOP
} // end namespace Belle2
