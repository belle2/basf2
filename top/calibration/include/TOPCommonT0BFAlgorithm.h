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
     * Algorithm for common T0 calibration with a fit of bunch finder residuals.
     * method BF
     */
    class TOPCommonT0BFAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor */
      TOPCommonT0BFAlgorithm();

      /** Destructor */
      virtual ~TOPCommonT0BFAlgorithm() {}

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
       * Sets minimal fitted offset uncertainty to declare this calibration as c_OK
       * @param minError minimal uncertainty [ns]
       */
      void setMinError(double minError) {m_minError = minError;}

    private:

      /**
       * algorithm implementation
       */
      virtual EResult calibrate() final;

      /**
       * Returns histogram to be fitted
       */
      std::shared_ptr<TH1F> getHistogram();

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

      // algorithm parameters
      int m_minEntries = 10; /**< minimal number of histogram entries to perform fit */
      double m_sigmaCoreInit = 0.060; /**< core gaussian sigma [ns] */
      double m_sigmaTailInit = 0.120; /**< tail gaussian sigma [ns] */
      double m_tailFractInit = 0.20; /**< fraction of tail gaussian */
      int m_cutoffEntries = 100; /**< cutoff entries for single/double gaussian fit */
      double m_minError = 0.020; /**< minimal commonT0 uncertainty [ns] to declare c_OK */

      // ntuple variables
      int m_expNo = 0; /**< experiment number */
      int m_runNo = 0; /**< first run number */
      int m_runLast = 0; /**< last run number */
      float m_fittedOffset = 0; /**< fitted offset */
      float m_offset = 0; /**< wrap-around of fitted offset (= common T0) */
      float m_offsetError = 0; /**< error on fitted offset */
      float m_sigmaCore = 0; /**< core gaussian sigma */
      float m_sigmaTail = 0; /**< tail gaussian sigma (set to 0 for single gauss fit) */
      float m_tailFract = 0; /**< tail fraction (set to 0 for single gauss fit) */
      float m_chi2 = 0;  /**< normalized chi^2 */
      float m_integral = 0; /**< number of fitted signal events */
      int m_numEvents = 0; /**< number of all events in the histogram */
      int m_fitStatus = -1; /**< fit status */

      // other
      double m_bunchTimeSep = 0; /**< bunch separation in time [ns] */

    };

  } // end namespace TOP
} // end namespace Belle2
