/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>
#include <TFile.h>
#include <top/dbobjects/TOPCalChannelMask.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Algorithm for masking of dead and hot channels
     */
    class TOPChannelMaskAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor */
      TOPChannelMaskAlgorithm();

      /** Destructor */
      virtual ~TOPChannelMaskAlgorithm() {}

      /**
       * Sets minimal average channel statistics needed to provide calibration
       * @param minHits minimal number of hits per channel (on average)
       */
      void setMinHits(double minHits) {m_minHits = minHits;}

      /**
       * Sets a projection band in y for window_vs_asic histogram which defines a range of un-corrupted window numbers
       * @param minWindow band lower limit
       * @param maxWindow band upper limit
       */
      void setWindowBand(int minWindow, int maxWindow)
      {
        m_minWindow = minWindow;
        m_maxWindow = maxWindow;
      }

    private:

      /**
       * algorithm implementation
       */
      virtual EResult calibrate() final;

      // algorithm parameters
      double m_minHits = 50; /**< minimal number of hits per channel needed for calibration */
      int m_minWindow = 215; /**< band lower limit in window_vs_asic */
      int m_maxWindow = 235; /**< band upper limit in window_vs_asic */

      // output file
      TFile* m_file = 0; /**< root file */

      // output DB object
      TOPCalChannelMask* m_channelMask = 0; /**< masks of dead and hot channels */

    };

  } // end namespace TOP
} // end namespace Belle2

