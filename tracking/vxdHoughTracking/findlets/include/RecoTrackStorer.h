/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/datastore/StoreArray.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

#include <string>
#include <vector>

namespace Belle2 {
  class ModuleParamList;
  class SpacePointTrackCand;
  class SpacePoint;
  class RecoTrack;
  class SVDCluster;

  namespace vxdHoughTracking {

    /// Store RecoTracks into StoreArray
    class RecoTrackStorer : public TrackFindingCDC::Findlet<SpacePointTrackCand, const SpacePoint* const> {
      /// Parent class
      using Super = TrackFindingCDC::Findlet<SpacePointTrackCand, const SpacePoint* const>;

    public:
      /// Constructor
      RecoTrackStorer();

      /// Default destructor
      ~RecoTrackStorer();

      /// Expose the parameters of the sub findlets.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Create the store arrays
      void initialize() override;

      /// Begin run
      void beginRun() override;

      /// Reset internal vectors
      void beginEvent() override;

      /// Store the finishey SpacePointTrackCands into RecoTracks and tag the SpacePoints
      void apply(std::vector<SpacePointTrackCand>& finishedResults,
                 const std::vector<const SpacePoint*>& spacePoints) override;

    private:
      /// StoreArray name of the output Track StoreArray
      std::string m_RecoTracksStoreArrayName = "SVDHoughRecoTracks";
      /// StoreArray name of the SVDCluster StoreArray
      std::string m_SVDClustersStoreArrayName = "SVDClusters";
      /// StoreArray name of the SpacePointTrackCandidate StoreArray
      std::string m_SVDSpacePointTrackCandsStoreArrayName = "SVDHoughSpacePointTrackCands";

      /// Output RecoTracks Store Array
      StoreArray<RecoTrack> m_storeRecoTracks;

      /// Output SpacePointTrackCand Store Array
      StoreArray<SpacePointTrackCand> m_storeSpacePointTrackCands;

      /// Store the used clusters in the results
      std::set<const SVDCluster*> m_usedClusters;
      /// Store the used space points in the results
      std::set<const SpacePoint*> m_usedSpacePoints;

      /// pointer to the selected QualityEstimator
      std::unique_ptr<QualityEstimatorBase> m_estimator;
      /// sets the name of the expected StoreArray containing MCRecoTracks. Only required for MCInfo method
      std::string m_MCRecoTracksStoreArrayName = "MCRecoTracks";
      /// Only required for MCInfo method
      bool m_MCStrictQualityEstimator = true;
      /// Identifier which estimation method to use. Valid identifiers are:
      /// mcInfo, circleFit, tripletFit, helixFit
      std::string m_EstimationMethod = "helixFit";
    };

  }
}
