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
#include <vector>

namespace Belle2 {
  namespace TOP {

    /**
     * Calibration algorithm for carrier shifts of BS13d.
     */
    class TOPAsicShiftsBS13dAlgorithm : public CalibrationAlgorithm {

    public:

      /** Constructor */
      TOPAsicShiftsBS13dAlgorithm();

      /** Destructor */
      virtual ~TOPAsicShiftsBS13dAlgorithm() {}

      /**
       * Sets minimal function value
       * @param minVal minimal value
       */
      void setMinValue(double minVal) {m_minVal = minVal;}

      /**
       * Sets size of the window on reference time distribution.
       * To remove window set size to 0.
       * @param size window size
       */
      void setWindowSize(int size) {m_winSize = size;}

      /**
       * Sets the search range
       * @param begin lower limit
       * @param end upper limit + 1
       */
      void setShiftRange(int begin, int end) {m_shiftBegin = begin; m_shiftEnd = end;}

      /**
       * Set minimal significance of the result to declare calibration as OK
       * @param s significance in number of sigma's
       */
      void setMinimalSignificance(double s) {m_minSignificance = s;}

    private:

      /**
       * algorithm implementation
       */
      virtual EResult calibrate() final;

      /**
       * Sets the position of a window containing maximum number of entries
       * of reference time distribution.
       */
      void setWindow();

      /**
       * Returns measured time distribution at x or minVal for outside-range or empty bins
       * @param x 0-based histogram bin number
       */
      double fun(int x);

      /**
       * Returns normalized time distribution at a given shift (PDF)
       * @param shift shift
       */
      std::vector<double> getPDF(int shift);

      /**
       * Returns log likelihood of a histogram with respect to PDF at a given shift
       * @param h histogram
       * @param shift shift
       */
      double logL(std::shared_ptr<TH1F> h, int shift);

      // parameters
      double m_minVal = 1.0; /**< minimal function value */
      int m_shiftBegin = -20; /**< shift range: lower limit */
      int m_shiftEnd = 20;  /**< shift range: upper limit + 1 */
      double m_minSignificance = 5; /**< minimal result significance to declare c_OK */

      // other
      std::vector<double> m_timeReference; /**< reference time distribution */
      int m_winSize = 21; /**< size of the window on reference time distribution */
      int m_i0 = 0; /**< first bin of the window on reference time distribution */
      int m_i1 = 0; /**< last bin of the window on reference time distribution */
      std::vector<int> m_lastEntries; /**< number of histogram entries */

    };

  } // end namespace TOP
} // end namespace Belle2
