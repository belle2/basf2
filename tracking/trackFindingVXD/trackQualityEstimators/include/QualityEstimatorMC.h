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
  /// Class implementing the algorithm used for the MC based quality estimation
  class QualityEstimatorMC : public QualityEstimatorBase {
  public:
    // some typedefs to increase readability
    typedef int MCRecoTrackIndex; /**< typedef for MCRecoTrackIndex */
    typedef int NMatches; /**< typedef for counter of number of matches */
    typedef std::pair<MCRecoTrackIndex, NMatches> MatchInfo; /**< typedef for MatchInfo */

    /** Constructor
     * @param mcRecoTracksStoreArrayName : Name of the MCRecoTracks StoreArray
     * @param strictQualityIndicator : boolean whether to perform strict estimation
     */
    QualityEstimatorMC(std::string mcRecoTracksStoreArrayName = "MCRecoTracks",
                       bool strictQualityIndicator = true):
      QualityEstimatorBase(), m_strictQualityIndicator(strictQualityIndicator), m_mcRecoTracksStoreArrayName(mcRecoTracksStoreArrayName)
    { };

    /** Performing MC based quality estimation
     * @param measurements : SPs of the track candidate to be evaluate
     * @return quality indicator value
     */
    virtual double estimateQuality(std::vector<SpacePoint const*> const& measurements) final;

    /** additionally return momentum_truth if it is a perfect match to a single MCRecoTrack */
    virtual QualityEstimationResults estimateQualityAndProperties(std::vector<SpacePoint const*> const& measurements) override final;

    /** Setter for StoreArray names of SVD and PXD clusters
     * @param svdClustersName : SVD cluster StoreArray name
     * @param pxdClustersName : PXD cluster StoreArray name
     */
    void setClustersNames(std::string svdClustersName, std::string pxdClustersName)
    { m_svdClustersName = svdClustersName; m_pxdClustersName = pxdClustersName; };

  protected:
    /** Get MCRecoTrack index of best matching tracks and number of matched MC clusters
     * which can be matched to the given track candidate
     * @param measurements : SPs to be evaluated
     * @return MatchInfo containing the ID of the best matching MCRecoTracks and number of matched clusters
     */
    MatchInfo getBestMatchToMCClusters(std::vector<SpacePoint const*> const& measurements);

    /** Calculate MC qualityIndicator based on MatchInfo of best matched MCRecoTrack
     * @param nClusters : number of clusters of the track candidate
     * @param match : MatchInfo for the best match
     * @return qualityIndicator
     */
    double calculateQualityIndicator(int nClusters, MatchInfo& match);

    // parameters
    /** If true only SPTCs containing SVDClusters corresponding to a single MCRecoTrack get a QI != 0.
     *  If a SVDCluster corresponds to several MCRecoTracks it is still valid as long as the correct MCRecoTrack is one of them.
     */
    bool m_strictQualityIndicator;

    // module members
    std::string m_mcRecoTracksStoreArrayName; /**< MCRecoTracks StoreArray name */
    std::string m_svdClustersName = ""; /**< SVD clusters StoreArray name */
    std::string m_pxdClustersName = ""; /**< PXD clusters StoreArray name */

    /** stores the current match for optional return values */
    MatchInfo m_match;

    /** the storeArray for RecoTracks as member */
    StoreArray<RecoTrack> m_mcRecoTracks;
  };
}
