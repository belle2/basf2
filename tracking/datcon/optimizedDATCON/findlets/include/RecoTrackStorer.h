/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/datastore/StoreArray.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

#include <string>
#include <vector>

namespace Belle2 {
  class SpacePointTrackCand;
  class SpacePoint;
  class RecoTrack;
  class SVDCluster;
  class ModuleParamList;

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
    /// StoreArray name of the input Track Store Array
    std::string m_param_RecoTracksStoreArrayName = "DATCONRecoTracks";

    /// Output RecoTracks Store Array
    StoreArray<RecoTrack> m_storeRecoTracks;

    /// Store the used clusters in the results
    std::set<const SVDCluster*> m_usedClusters;
    /// Store the used space points in the results
    std::set<const SpacePoint*> m_usedSpacePoints;

    /// pointer to the selected QualityEstimator
    std::unique_ptr<QualityEstimatorBase> m_estimator;
    /// sets the name of the expected StoreArray containing MCRecoTracks. Only required for MCInfo method
    std::string m_param_MCRecoTracksStoreArrayName = "MCRecoTracks";
    /// Only required for MCInfo method
    bool m_param_MCStrictQualityEstimator = true;
    /// Identifier which estimation method to use. Valid identifiers are:
    /// mcInfo, circleFit, tripletFit, helixFit
    std::string m_param_EstimationMethod = "helixFit";
  };
}
