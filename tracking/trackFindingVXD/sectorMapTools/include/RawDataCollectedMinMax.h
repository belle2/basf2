/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/sectorMapTools/MinMaxCollector.h>
#include <framework/logging/Logger.h>

#include <vector>
#include <utility> // std::pair
#include <limits>       // std::numeric_limits
#include <algorithm> // std::sort


namespace Belle2 {

  /// takes care of collecting raw data and staying below RAM-threshold.
  class RawDataCollectedMinMax {
  protected:
    unsigned m_currentSize; ///< the current size of the data sample.
    unsigned m_fillIntermediateThreshold; ///< an internal threshold taking care of collecting intermediate results during sample collection
    std::pair<double, double> m_minMaxQuantiles; ///< the quantiles to be collected in the end (defined in [0;1])
    std::vector<std::pair<double, double>> m_intermediateValues; ///< collects intermediate threshold if expected size is too big.
    MinMaxCollector<double> m_collector; ///< collects raw data in an RAM-saving way.
  public:

    /// constructor. please use for quantiles [min, max] min ~0 & max ~1 (range 0-1)
    RawDataCollectedMinMax(unsigned expectedSize,
                           std::pair<double, double> quantiles,
                           unsigned maxSizeThreshold = 100000) :
      m_currentSize(0),
      m_fillIntermediateThreshold(std::numeric_limits<unsigned>::max()),
      m_minMaxQuantiles(quantiles),
      m_collector((quantiles.first > (1. - quantiles.second) ? quantiles.first * 2. : (1. - quantiles.second) * 2.))
    {
      if (double(expectedSize) / (double(maxSizeThreshold) * 0.05) > double(maxSizeThreshold))
      { B2FATAL("RawDataCollectedMinMax: expected data to big, can not execute!"); }

      if (maxSizeThreshold < expectedSize) {
        m_fillIntermediateThreshold = maxSizeThreshold / 10;
      }
    }

    /// adds value to collector.
    void add(double newVal)
    {
      m_collector.append(newVal);
      m_currentSize++;

      // if threshold reached, collect results and fill into intermediate value-container:
      if (m_collector.totalSize() > m_fillIntermediateThreshold) {
        std::pair<double, double> results = m_collector.getMinMax(m_minMaxQuantiles.first, m_minMaxQuantiles.second);
        m_intermediateValues.push_back(std::move(results));
        m_collector.clear();
      }
    }

    /// returns current sample size (which is not the actual size of the container).
    unsigned getSampleSize() const { return m_currentSize; }

    /// returns current best estimates for min and max cuts.
    std::pair<double, double> getMinMax()
    {
      if (m_intermediateValues.empty()) {
        return m_collector.getMinMax(m_minMaxQuantiles.first, m_minMaxQuantiles.second);
      }

      // issue: m_collector-sample could be too small and therefore distort results for small intermediateValue-samples. Therefore neglect m_collector for that case.
      if (m_intermediateValues.size() == 1) {
        return { m_intermediateValues.at(0).first, m_intermediateValues.at(0).second};
      }
      if (m_intermediateValues.size() == 2) {
        return {
          0.5 * (m_intermediateValues.at(0).first + m_intermediateValues.at(1).first),
          0.5 * (m_intermediateValues.at(0).second + m_intermediateValues.at(1).second) };
      }

      if (!m_collector.empty()) {
        std::pair<double, double> results = m_collector.getMinMax(m_minMaxQuantiles.first, m_minMaxQuantiles.second);
        m_intermediateValues.push_back(results);
      }

      unsigned index = std::floor(double(m_intermediateValues.size()) * 0.5);
      double min, max;

      std::sort(m_intermediateValues.begin(), m_intermediateValues.end(),
                [](const std::pair<double, double>& a, const std::pair<double, double>& b) -> bool { return a.first < b.first; });
      min = m_intermediateValues.at(index).first;

      std::sort(m_intermediateValues.begin(), m_intermediateValues.end(),
                [](const std::pair<double, double>& a, const std::pair<double, double>& b) -> bool { return a.second < b.second; });
      max = m_intermediateValues.at(index).second;

      return {min, max};
    }
  };
}

