/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/numerics/Median.h>

#include <tracking/trackFindingCDC/numerics/WeightComperator.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <framework/logging/Logger.h>

#include <cmath>

using namespace Belle2;

double TrackFindingCDC::median(std::vector<double> values)
{
  auto notfinite = [](double v) { return not std::isfinite(v); };
  erase_remove_if(values, notfinite);

  int n = values.size();

  if (n == 0) return NAN;

  std::nth_element(values.begin(), values.begin() + n / 2, values.end());
  if (n % 2 == 0) {
    // For an even number of values the median is the average of the two central values.
    double upper = values[n / 2];
    double lower = *std::max_element(values.begin(), values.begin() + n / 2);
    return (lower + upper) / 2;
  } else {
    return values[n / 2];
  }
}

double TrackFindingCDC::weightedMedian(std::vector<WithWeight<double> > weightedValues)
{
  auto notfinite = [](const WithWeight<double>& weightedValue) {
    return not std::isfinite(weightedValue) or not std::isfinite(weightedValue.getWeight());
  };
  erase_remove_if(weightedValues, notfinite);

  if (weightedValues.empty()) return NAN;

  std::sort(weightedValues.begin(), weightedValues.end());

  /// Accumulate the weight overall sorted values
  Weight totalWeight = 0;
  for (WithWeight<double>& weightedValue : weightedValues) {
    totalWeight += weightedValue.getWeight();
    weightedValue.setWeight(totalWeight);
  }

  auto it = std::partition_point(weightedValues.begin(),
                                 weightedValues.end(),
                                 GetWeight() < totalWeight / 2.0);
  return it == weightedValues.end() ? weightedValues.back() : *it;
}
