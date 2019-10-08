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
#include <TH2F.h>
#include <TH1D.h>
#include <TFile.h>
#include <TTree.h>
#include <top/dbobjects/TOPCalChannelPulseHeight.h>
#include <top/dbobjects/TOPCalChannelThresholdEff.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Algorithm for pulse-height and threshold efficiency calibration
     */
    class TOPPulseHeightAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor */
      TOPPulseHeightAlgorithm();

      /** Destructor */
      virtual ~TOPPulseHeightAlgorithm() {}

      /**
       * Sets minimal number of histogram entries to perform a fit
       */
      void setMinEntries(int minEntries) {m_minEntries = minEntries;}

      /**
       * Sets the lower bound of the fit range
       * @param xmin lower bound [ADC counts]
       */
      void setXmin(double xmin) {m_xmin = xmin;}

      /**
       * Sets minimal fraction of calibrated channels to declare this calibration as c_OK
       * @param minFract minimal fraction of calibrated channels
       */
      void setMinFracCalibrated(double minFract) {m_minCalibrated = minFract;}

    private:

      /**
       * algorithm implementation
       */
      virtual EResult calibrate() final;

      /**
       * Fit pulse-height distributions of a single module
       * @param h 2D histogram of pulse-height vs channel
       * @return number of successfully fitted channels
       */
      int fitChannels(std::shared_ptr<TH2F> h);

      /**
       * Fit pulse-height distribution with P(x) = (x/x0)^p1 * exp(-(x/x0)^p2) and p1 = 1.
       * Fitting with floating p1 found to be unstabe.
       * @param h histogram
       * @return fit status
       */
      int fitPulseHeight(TH1D* h);

      /**
       * Calculate and return threshold efficiency
       * @param k histogram
       * @param func fitted function
       * @return threshold efficiency
       */
      double getEfficiency(TH1D* h, TF1* func);

      // algorithm parameters
      int m_minEntries = 100; /**< minimal number of histogram entries to perform fit */
      double m_xmin = 100.0; /**< fit range lower limit [ADC counts] */
      double m_minCalibrated = 0.90; /**< min. fracton of calibrated channels for c_OK */

      // ntuple variables
      int m_slot = 0; /**< module ID */
      unsigned m_channel = 0; /**< channel ID */
      int m_numPhot = 0; /**< number of photons (histogram entries) */
      float m_x0 = 0; /**< fitted distribution parameter x0 [ADC counts] */
      float m_p1 = 0; /**< fitted distribution parameter p1 */
      float m_p2 = 0; /**< fitted distribution parameter p2 */
      float m_x0err = 0; /**< error on x0 [ADC counts] */
      float m_p1err = 0; /**< error on p1 */
      float m_p2err = 0; /**< error on p2 */
      float m_effi = 0; /**< threshold efficiency */
      float m_mean = 0; /**< fitted distribution mean value */
      float m_chi2 = 0;  /**< chi^2 */
      int m_ndf = 0; /**< NDF */
      int m_fitStatus = -1; /**< fit status */

      // output file and tree
      TFile* m_file = 0; /**< root file */
      TTree* m_tree = 0; /**< output ntuple */

      // output DB objects
      TOPCalChannelPulseHeight* m_pulseHeight = 0; /**< parametrized PH distributions */
      TOPCalChannelThresholdEff* m_thresholdEffi = 0; /**< threshold efficiencies */

    };

  } // end namespace TOP
} // end namespace Belle2
