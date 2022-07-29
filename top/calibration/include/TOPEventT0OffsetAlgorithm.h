/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Algorithm for event T0 offset calibration
     */
    class TOPEventT0OffsetAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor */
      TOPEventT0OffsetAlgorithm();

      /** Destructor */
      virtual ~TOPEventT0OffsetAlgorithm() {}

      /**
       * Sets minimal number of histogram entries to perform a fit
       * @param minEntries minimal number of histogram entries
       */
      void setMinEntries(int minEntries) {m_minEntries = minEntries;}

      /**
       * Sets initial value of the Gaussian width
       * @param sigma Gaussian sigma [ns]
       */
      void setSigma(double sigma) {m_sigma = sigma;}

    private:

      /**
       * algorithm implementation
       */
      virtual EResult calibrate() final;

      int m_minEntries = 100; /**< minimal number of entries to perform fit */
      double m_sigma = 1.0;   /**< initial value for Gaussian width [ns] */

    };

  } // end namespace TOP
} // end namespace Belle2
