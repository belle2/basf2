/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/ckf/findlets/base/StoreArrayHandler.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  /**
   * Derived store array handler for CDC RecoTrack seeds and SpacePoints.
   *
   * This findlet is responsible for the interface between the DataStore and the CKF modules:
   *  * to write back the found VXD tracks only and the merged tracks (CDC + VXD) (apply)
   */
  template <class AResultObject>
  class CDCTrackSpacePointStoreArrayHandler : public TrackFindingCDC::Findlet<const AResultObject> {
    using Super = TrackFindingCDC::Findlet<const AResultObject>;

  public:
    CDCTrackSpacePointStoreArrayHandler() : Super()
    {
      this->addProcessingSignalListener(&m_storeArrayHandler);
    }

    /// Expose the parameters of the findlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      moduleParamList->addParameter("exportTracks", m_param_exportTracks, "Export the result tracks into a StoreArray.",
                                    m_param_exportTracks);

      m_storeArrayHandler.exposeParameters(moduleParamList, prefix);
    }

    /**
     * Write back the found tracks (VXD only and the merged ones).
     */
    void apply(const std::vector<AResultObject>& cdcTracksWithMatchedSpacePoints) override
    {
      // Create a list of cdc-track - svd cluster list pairs
      std::vector<std::pair<const RecoTrack*, std::pair<TVector3, std::vector<const SVDCluster*>>>> seedsWithPositionAndHits;
      seedsWithPositionAndHits.reserve(cdcTracksWithMatchedSpacePoints.size());

      // Create new VXD tracks out of the found VXD space points and store them into a store array
      for (const auto& cdcTrackWithMatchedSpacePoints : cdcTracksWithMatchedSpacePoints) {
        RecoTrack* cdcRecoTrack = cdcTrackWithMatchedSpacePoints.first;
        if (not cdcRecoTrack) {
          continue;
        }

        const std::vector<const SpacePoint*> matchedSpacePoints = cdcTrackWithMatchedSpacePoints.second;
        if (matchedSpacePoints.empty()) {
          continue;
        }

        const TVector3& vxdPosition = matchedSpacePoints.front()->getPosition();
        seedsWithPositionAndHits.emplace_back(cdcRecoTrack, std::make_pair(vxdPosition, std::vector<const SVDCluster*>()));
        auto& hits = seedsWithPositionAndHits.back().second.second;

        for (const auto& spacePoint : matchedSpacePoints) {
          if (spacePoint) {
            const auto& relatedClusters = spacePoint->getRelationsWith<SVDCluster>();
            for (const SVDCluster& relatedCluster : relatedClusters) {
              hits.push_back(&relatedCluster);
            }
          }
        }
      }

      m_storeArrayHandler.combine(seedsWithPositionAndHits);
    }

  private:
    StoreArrayHandler m_storeArrayHandler;

    /** Export the tracks or not */
    bool m_param_exportTracks = true;
  };
}
