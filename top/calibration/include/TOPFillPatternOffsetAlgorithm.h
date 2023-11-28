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
     * Algorithm for calibration of fill pattern offset
     */
    class TOPFillPatternOffsetAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor */
      TOPFillPatternOffsetAlgorithm();

      /** Destructor */
      virtual ~TOPFillPatternOffsetAlgorithm() {}

      /**
       * Sets minimal number of histogram entries to perform calibration
       * @param minEntries minimal number of histogram entries
       */
      void setMinEntries(int minEntries) {m_minEntries = minEntries;}

      /**
       * Sets signal fraction used in PDF
       * @param p signal fraction
       */
      void setSignalFraction(double p) {m_p = p;}

      /**
       * Sets minimal fraction of matched buckets to save calibration constants
       * @param fract minimal fraction
       */
      void setMinimalFraction(double fract) {m_minFract = fract;}

    private:

      /**
       * algorithm implementation
       */
      virtual EResult calibrate() final;

      /**
       * Returns chi2 = -2 logL of circularly shifted pattern at a give shift i0
       * @param recBuckets reconstructed buckets
       * @param fillPattern fill pattern
       * @param i0 shift
       * @return chi2
       */
      double Chi2(TH1F* recBuckets, TH1F* fillPattern, int i0);

      /**
       * Returns chi2 values at all possible circular shifts
       * @param recBuckets reconstructed buckets
       * @param fillPattern fill pattern
       * @return a histogram of chi2 values
       */
      TH1F* getChi2Histogram(TH1F* recBuckets, TH1F* fillPattern);

      /**
       * Returns fraction of reconstructed buckets matched with filled buckets after accounting for the offset.
       * @param recBuckets reconstructed buckets
       * @param fillPattern fill pattern
       * @param offset fill pattern offset
       * return fraction of reconstructed buckets matched with filled buckets
       */
      double getFraction(TH1F* recBuckets, TH1F* fillPattern, int offset);

      int m_minEntries = 100; /**< minimal number of entries to perform calibration */
      double m_p = 0.99;      /**< signal fraction for PDF definition */
      double m_minFract = 0.9; /**< minimal fraction of matched buckets to save calibration */

    };

  } // end namespace TOP
} // end namespace Belle2
