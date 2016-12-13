/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/CDCRobustSZFitter.h>

#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <tracking/trackFindingCDC/fitting/CDCSZObservations.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <functional>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace Eigen;

namespace {
  double median(std::vector<double> values)
  {
    auto notfinite = [](double v) { return not std::isfinite(v); };
    erase_remove_if(values, notfinite);

    int n = values.size();
    if (n == 0) return NAN;
    std::nth_element(values.begin(), values.begin() + n / 2, values.end());
    if (n % 2) {
      // For an even number of values the median is the average of the two central values.
      double upper = values[n / 2];
      double lower = *std::max_element(values.begin(), values.begin() + n / 2);
      return (lower + upper) / 2;
    } else {
      return values[n / 2];
    }
  }

  double weightedMedian(std::vector<WithWeight<double> > weightedValues)
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
    auto it = std::upper_bound(weightedValues.begin(), weightedValues.end(), totalWeight / 2);
    return it == weightedValues.end() ? weightedValues.back() : *it;
  }
}

CDCTrajectorySZ CDCRobustSZFitter::fitUsingSimplifiedTheilSen(const CDCSZObservations& observationsSZ) const
{
  // This seems to be some other algorithm


  CDCTrajectorySZ trajectorySZ;
  CDCSZFitter szFitter;

  if (observationsSZ.size() > 4) {
    CDCSZObservations observationsSZFiltered;

    double meanTanLambda = 0;
    double meanStartZ = 0;

    for (unsigned int i = 0; i < observationsSZ.size(); i++) {
      for (unsigned int j = 0; j < observationsSZ.size(); j++) {
        if (i != j) {
          observationsSZFiltered.fill(observationsSZ.getS(j),
                                      observationsSZ.getZ(j),
                                      observationsSZ.getWeight(j));
        }
      } // for j

      szFitter.update(trajectorySZ, observationsSZFiltered);
      meanTanLambda += trajectorySZ.getTanLambda();
      meanStartZ += trajectorySZ.getZ0();
    } // for i

    return CDCTrajectorySZ(meanTanLambda / observationsSZ.size(),
                           meanStartZ / observationsSZ.size());
  } else {
    return CDCTrajectorySZ::basicAssumption();
  }
}

CDCTrajectorySZ CDCRobustSZFitter::fitTheilSen(const CDCSZObservations& szObservations) const
{
  std::vector<double> tanLambdas;
  tanLambdas.reserve(szObservations.size() * (szObservations.size() - 1) / 2);
  for (unsigned int i = 0; i < szObservations.size(); i++) {
    for (unsigned int j = i + 1; j < szObservations.size(); j++) {
      double s1 = szObservations.getS(i);
      double s2 = szObservations.getS(j);
      if (s1 == s2) continue;
      double z1 = szObservations.getZ(i);
      double z2 = szObservations.getZ(j);
      tanLambdas.push_back((z2 - z1) / (s2 - s1));
    }
  }

  const double tanLambda = median(std::move(tanLambdas));
  const double z0 = getMedianZ0(szObservations, tanLambda);

  CDCTrajectorySZ trajectorySZ(tanLambda, z0);
  trajectorySZ.setNDF(szObservations.size());
  return trajectorySZ;
}

CDCTrajectorySZ CDCRobustSZFitter::fitWeightedTheilSen(const CDCSZObservations& szObservations) const
{
  std::vector<WithWeight<double> > weightedTanLambdas;
  Weight totalWeight = 0;
  weightedTanLambdas.reserve(szObservations.size() * (szObservations.size() - 1) / 2);
  for (unsigned int i = 0; i < szObservations.size(); i++) {
    for (unsigned int j = i + 1; j < szObservations.size(); j++) {
      double s1 = szObservations.getS(i);
      double s2 = szObservations.getS(j);
      if (s1 == s2) continue;
      double z1 = szObservations.getZ(i);
      double z2 = szObservations.getZ(j);

      double w1 = szObservations.getWeight(i);
      double w2 = szObservations.getWeight(j);

      // Longer legs receive proportionally longer weight.
      Weight weight = std::abs(s2 - s1) * hypot2(w1, w2);
      weightedTanLambdas.emplace_back((z2 - z1) / (s2 - s1),  weight);
      totalWeight += weight;
    }
  }

  for (WithWeight<double>& weightedTanLambda : weightedTanLambdas) {
    weightedTanLambda.weight() /= totalWeight;
  }

  const double tanLambda = weightedMedian(std::move(weightedTanLambdas));
  const double z0 = getMedianZ0(szObservations, tanLambda);

  CDCTrajectorySZ trajectorySZ(tanLambda, z0);
  trajectorySZ.setNDF(szObservations.size());
  return trajectorySZ;
}

double CDCRobustSZFitter::getMedianZ0(const CDCSZObservations& szObservations, double tanLambda) const
{
  std::vector<double> z0s;
  z0s.reserve(szObservations.size());
  for (unsigned int i = 0; i < szObservations.size(); i++) {
    double s = szObservations.getS(i);
    double z = szObservations.getZ(i);
    z0s.push_back(z - s * tanLambda);
  }
  const double z0 = median(std::move(z0s));
  return z0;
}
