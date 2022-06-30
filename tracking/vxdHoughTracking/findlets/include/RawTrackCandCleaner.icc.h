/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdHoughTracking/findlets/RawTrackCandCleaner.dcl.h>
#include <tracking/vxdHoughTracking/utilities/SVDHoughTrackingHelpers.h>

#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/vxdHoughTracking/entities/VXDHoughState.h>
#include <tracking/vxdHoughTracking/filters/relations/LayerRelationFilter.icc.h>
#include <tracking/vxdHoughTracking/findlets/SVDHoughTrackingTreeSearcher.icc.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <vxd/dataobjects/VxdID.h>

namespace Belle2::vxdHoughTracking {

  template<class AHit>
  RawTrackCandCleaner<AHit>::~RawTrackCandCleaner() = default;

  template<class AHit>
  RawTrackCandCleaner<AHit>::RawTrackCandCleaner() : Super()
  {
    Super::addProcessingSignalListener(&m_relationCreator);
    Super::addProcessingSignalListener(&m_treeSearcher);
    Super::addProcessingSignalListener(&m_resultRefiner);
  }

  template<class AHit>
  void RawTrackCandCleaner<AHit>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    Super::exposeParameters(moduleParamList, prefix);
    m_relationCreator.exposeParameters(moduleParamList, prefix);
    m_treeSearcher.exposeParameters(moduleParamList, prefix);
    m_resultRefiner.exposeParameters(moduleParamList, prefix);

    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maxRelations"), m_maxRelations,
                                  "Maximum number of relations allowed for entering tree search.", m_maxRelations);
  }

  template<class AHit>
  void RawTrackCandCleaner<AHit>::initialize()
  {
    Super::initialize();
  }

  template<class AHit>
  void RawTrackCandCleaner<AHit>::apply(std::vector<std::vector<AHit*>>& rawTrackCandidates,
                                        std::vector<SpacePointTrackCand>& trackCandidates)
  {
    uint family = 0; // family of the SpacePointTrackCands
    for (auto& rawTrackCand : rawTrackCandidates) {

      // If the capacity of a std::vector is too large, start with a fresh one.
      // Since std::vector.shrink() or std::vector.shrink_to_fit() not necessarily reduce the capacity in the desired way,
      // create a temporary vector of the same type and swap them to use the vector at the new location afterwards.
      checkResizeClear<TrackFindingCDC::WeightedRelation<AHit>>(m_relations, 8192);
      checkResizeClear<Result>(m_results, 8192);
      checkResizeClear<SpacePointTrackCand>(m_unfilteredResults, 8192);
      checkResizeClear<SpacePointTrackCand>(m_filteredResults, 8192);

      m_relationCreator.apply(rawTrackCand, m_relations);

      if (m_relations.size() > m_maxRelations) {
        m_relations.clear();
        continue;
      }

      m_treeSearcher.apply(rawTrackCand, m_relations, m_results);

      m_unfilteredResults.reserve(m_results.size());
      for (const std::vector<TrackFindingCDC::WithWeight<const AHit*>>& result : m_results) {
        std::vector<const SpacePoint*> spacePointsInResult;
        spacePointsInResult.reserve(result.size());
        for (const TrackFindingCDC::WithWeight<const AHit*>& hit : result) {
          spacePointsInResult.emplace_back(hit->getHit());
        }
        std::sort(spacePointsInResult.begin(), spacePointsInResult.end(), [](const SpacePoint * a, const SpacePoint * b) {
          return
            (a->getVxdID().getLayerNumber() < b->getVxdID().getLayerNumber()) or
            (a->getVxdID().getLayerNumber() == b->getVxdID().getLayerNumber()
             and a->getPosition().Perp() < b->getPosition().Perp());
        });
        m_unfilteredResults.emplace_back(spacePointsInResult);
      }

      for (auto aTC : m_unfilteredResults) {
        aTC.setFamily(family);
      }

      m_resultRefiner.apply(m_unfilteredResults, m_filteredResults);

      for (const SpacePointTrackCand& trackCand : m_filteredResults) {
        trackCandidates.emplace_back(trackCand);
      }

      family++;
    }
  }

}
