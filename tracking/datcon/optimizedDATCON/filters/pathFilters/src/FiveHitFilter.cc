/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/FiveHitFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
FiveHitFilter::operator()(const BasePathFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const HitData*>>& previousHits = pair.first;
//   HitData* currentHit = pair.second;

  // Do nothing if path is too short or too long
  if (previousHits.size() != 4) {
    return NAN;
  }

  std::vector<const SpacePoint*> spacePoints;
  spacePoints.reserve(previousHits.size() + 1);
  spacePoints.emplace_back(previousHits.at(0)->getHit());
  spacePoints.emplace_back(previousHits.at(1)->getHit());
  spacePoints.emplace_back(previousHits.at(2)->getHit());
  spacePoints.emplace_back(previousHits.at(3)->getHit());
  spacePoints.emplace_back(pair.second->getHit());
  const auto& estimatorResult = helixFitEstimator.estimateQualityAndProperties(spacePoints);

  const double absHelixPocaD = (estimatorResult.pocaD) ? fabs(*estimatorResult.pocaD) : 1e-6;
  const double chi2 = (estimatorResult.chiSquared) ? *estimatorResult.chiSquared : 1e6;

//   B2INFO("PocaD: " << absHelixPocaD << " chi2: " << chi2);

  if (absHelixPocaD > m_helixFitPocaDCut) {
    return NAN;
  }

//   return 1.0 / chi2;
  return estimatorResult.qualityIndicator;
}

void FiveHitFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "helixFitPocaDCut"), m_helixFitPocaDCut,
                                "Cut on the POCA difference in xy with the POCA obtained from a helix fit (tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit).",
                                m_helixFitPocaDCut);
}
