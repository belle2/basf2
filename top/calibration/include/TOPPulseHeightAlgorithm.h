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

    private:

      /**
       * algorithm implementation
       */
      virtual EResult calibrate() final;

      /**
       * Fit pulse-height distribution with P(x) = (x/x0)^p1 * exp(-(x/x0)^p2)
       * @param h histogram
       * @return fit status
       */
      int fitPulseHeight(std::shared_ptr<TH1F> h);

      // algorithm parameters
      int m_minEntries = 100; /**< minimal number of histogram entries to perform fit */
      double m_xmin = 100.0; /**< fit range lower limit [ADC counts] */

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
      float m_chi2 = 0;  /**< chi^2 */
      int m_ndf = 0; /**< NDF */
      int m_fitStatus = -1; /**< fit status */

    };

  } // end namespace TOP
} // end namespace Belle2
