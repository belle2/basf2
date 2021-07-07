/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#ifndef INCLUDE_GUARD_BELLE2_MVA_BINNING_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_BINNING_HEADER

#include <vector>

namespace Belle2 {
  namespace MVA {

    /**
     * Binning of a data distribution
     * Provides PDF and CDF values of the distribution per bin
     */
    class Binning {
    public:
      /**
       * Creates an empty binning with nBins
       * @param nBins number of bins
       */
      explicit Binning(unsigned int nBins = 0);

      /**
       * Gets the bin corresponding to the given datapoint.
       * There are no overlow/underflow bins, so data points outside the original range
       * are mapped to the first and last bin.
       * @param datapoint for which the bin is returned
       */
      unsigned int getBin(float datapoint) const;

      /**
       * Normalizes the PDF values, so their sum is 1
       */
      void normalizePDFs();

      /**
       * Calculates the CDF values from the pdf values, which are assumed to be normalized
       */
      void calculateCDFsFromPDFs();

      double m_signal_yield; /**< Signal yield in data distribution */
      double m_bckgrd_yield; /**< Background yield in data distribution */

      std::vector<float> m_signal_pdf; /**< Signal pdf of data distribution per bin */
      std::vector<float> m_signal_cdf; /**< Signal cdf of data distribution per bin */
      std::vector<float> m_bckgrd_pdf; /**< Background pdf of data distribution per bin */
      std::vector<float> m_bckgrd_cdf; /**< Background cdf of data distribution per bin */
      std::vector<float>
      m_boundaries; /**< Boundaries of data distribution, including minimum and maximum value as first and last boundary */

      /**
       * Create an equal frequency (aka equal-statistics) binning.
       * @param data data-points sampled from the distribution
       * @param weights weights for each data-point
       * @param isSignal per data point
       * @param nBins number of bins
       */
      static Binning CreateEqualFrequency(const std::vector<float>& data, const std::vector<float>& weights,
                                          const std::vector<bool>& isSignal, unsigned int nBins);

      /**
       * Create an equidistant binning.
       * @param data data-points sampled from the distribution
       * @param weights weights for each data-point
       * @param isSignal per data point
       * @param nBins number of bins
       */
      static Binning CreateEquidistant(const std::vector<float>& data, const std::vector<float>& weights,
                                       const std::vector<bool>& isSignal, unsigned int nBins);

    };

  }
}

#endif
