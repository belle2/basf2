/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/vtx/findlets/VTXRecoTrackRelator.h>
#include <tracking/ckf/vtx/entities/CKFToVTXResult.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>
#include <framework/core/ModuleParamList.h>

using namespace Belle2;

VTXRecoTrackRelator::VTXRecoTrackRelator()
{
  addProcessingSignalListener(&m_overlapFilter);
}

void VTXRecoTrackRelator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_overlapFilter.exposeParameters(moduleParamList, prefix);
}

void VTXRecoTrackRelator::apply(const std::vector<CKFToVTXResult>& results,
                                std::vector<TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>>& relationsCDCToVTX)
{
  for (const CKFToVTXResult& result : results) {
    const TrackFindingCDC::Weight weight = m_overlapFilter(result);
    if (not std::isnan(weight)) {
      const RecoTrack* relatedVTXTrack = result.getRelatedVTXRecoTrack();
      relationsCDCToVTX.emplace_back(result.getSeed(), weight, relatedVTXTrack);
    }
  }
}
