/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>
#include <Eigen/Sparse>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

namespace Belle2 {
  class QualityEstimatorMC : public QualityEstimatorBase {

  public:
    // some typedefs to increase readability
    typedef int MCRecoTrackIndex;
    typedef int NMatches;
    typedef std::pair<MCRecoTrackIndex, NMatches> MatchInfo;

    QualityEstimatorMC(std::string mcRecoTracksStoreArrayName = "MCRecoTracks",
                       bool strictQualityIndex = true):
      QualityEstimatorBase(), m_strictQualityIndex(strictQualityIndex), m_mcRecoTracksStoreArrayName(mcRecoTracksStoreArrayName)
    { };

    virtual double estimateQuality(std::vector<SpacePoint const*> const& measurements) final;

    /** additionally return momentum_truth if it is a perfect match to a single MCRecoTrack */
    virtual QualityEstimationResults estimateQualityAndProperties(std::vector<SpacePoint const*> const& measurements) override final;

    void setClustersNames(std::string svdClustersName, std::string pxdClustersName)
    { m_svdClustersName = svdClustersName; m_pxdClustersName = pxdClustersName; };

  protected:
    MatchInfo getBestMatchToMCClusters(std::vector<SpacePoint const*> const& measurements);
    double calculateQualityIndex(int nClusters, MatchInfo& match);

    // parameters

    /** If true only SPTCs containing SVDClusters corresponding to a single MCRecoTrack get a QI != 0.
     *  If a SVDCluster corresponds to several MCRecoTracks it is still valid as long as the correct MCRecoTrack is one of them.
     */
    bool m_strictQualityIndex;

    std::string m_mcRecoTracksStoreArrayName;
    // module members
    std::string m_svdClustersName = "";
    std::string m_pxdClustersName = "";

    /** stores the current match for optional return values */
    MatchInfo m_match;

    /** the storeArray for RecoTracks as member */
    StoreArray<RecoTrack> m_mcRecoTracks;
  };
}
