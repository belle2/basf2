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

#include <string>
#include <utility> // std::pair
#include <limits>       // std::numeric_limits


namespace Belle2 {

  /** small class for storing min and max. */
  class MinMax {
  protected:
    std::pair<double, double> m_minMax; /**< .first is min, .second is .max. */
  public:

    /** construtor, starts with max for min and min for max (will later be replaced by valid entries). */
    MinMax() :
      m_minMax( {std::numeric_limits< double >::max(), -std::numeric_limits< double >::max()}) {}

    /// reset to start values.
    void reset() { m_minMax = {std::numeric_limits< double >::max(), -std::numeric_limits< double >::max()}; }

    /** checks value and replaces old ones if new one is better. returns true if minMax was updated. */
    bool checkAndReplaceIfMinMax(double newVal)
    {
      bool wasAdded = false;
      if (newVal < m_minMax.first) { m_minMax.first = newVal; wasAdded = true; }
      if (m_minMax.second < newVal) { m_minMax.second = newVal; wasAdded = true; }
      return wasAdded;
    }

    double getMin() const { return m_minMax.first; } /**< returns smallest value stored so far. */

    double getMax() const { return m_minMax.second; } /**< returns biggest value stored so far. */

    std::string print() const
    {
      std::string min = m_minMax.first == std::numeric_limits< double >::max() ? "max<double>" : std::to_string(m_minMax.first);
      std::string max = m_minMax.second == -std::numeric_limits< double >::max() ? "min<double>" : std::to_string(m_minMax.second);
      return "min: " + min + ", max: " + max;
    } /**< print function returning results for min and max as a string. */
  };
}

