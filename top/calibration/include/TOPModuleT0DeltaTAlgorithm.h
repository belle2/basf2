/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
      TFitResultPtr fitSingleGaus(std::shared_ptr<TH1F> h);

      /**
       * Fit double gaus w/ same mean + constant
       * @param h histogram
       * @return fit status
       */
      TFitResultPtr fitDoubleGaus(std::shared_ptr<TH1F> h);

      // algorithm parameters
      int m_minEntries = 10; /**< minimal number of histogram entries to perform fit */
      double m_sigmaCoreInit = 0.120; /**< core gaussian sigma [ns] */
      double m_sigmaTailInit = 0.240; /**< tail gaussian sigma [ns] */
      double m_tailFractInit = 0.20; /**< fraction of tail gaussian */
      int m_cutoffEntries = 100; /**< cutoff entries for single/double gaussian fit */
      double m_minError = 0.100; /**< minimal commonT0 uncertainty [ns] to declare c_OK */

    };

  } // end namespace TOP
} // end namespace Belle2
