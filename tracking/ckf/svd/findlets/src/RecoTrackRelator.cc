/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/findlets/RecoTrackRelator.h>
#include <tracking/ckf/svd/entities/CKFToSVDResult.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>
#include <framework/core/ModuleParamList.h>

using namespace Belle2;

RecoTrackRelator::RecoTrackRelator()
{
  addProcessingSignalListener(&m_overlapFilter);
}

void RecoTrackRelator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_overlapFilter.exposeParameters(moduleParamList, prefix);
}

void RecoTrackRelator::apply(const std::vector<CKFToSVDResult>& results,
                             std::vector<TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>>& relationsCDCToSVD)
{
  for (const CKFToSVDResult& result : results) {
    const TrackFindingCDC::Weight weight = m_overlapFilter(result);
    if (not std::isnan(weight)) {
      const RecoTrack* relatedSVDTrack = result.getRelatedSVDRecoTrack();
      relationsCDCToSVD.emplace_back(result.getSeed(), weight, relatedSVDTrack);
    }
  }
}