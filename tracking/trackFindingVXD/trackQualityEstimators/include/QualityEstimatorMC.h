/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  /// Class implementing the algorithm used for the MC based quality estimation
  class QualityEstimatorMC : public QualityEstimatorBase {
  public:
    // some typedefs to increase readability
    typedef int MCRecoTrackIndex; /**< typedef for MCRecoTrackIndex */
    typedef unsigned int NMatches; /**< typedef for counter of number of matches */
    typedef std::pair<MCRecoTrackIndex, NMatches> MatchInfo; /**< typedef for MatchInfo */

    /** Constructor
     * @param mcRecoTracksStoreArrayName : Name of the MCRecoTracks StoreArray
     * @param strictQualityIndicator : boolean whether to perform strict estimation
     * @param mva_target : Boolean whether to perform quality estimation for MVA QE training.
     *                     This overwrites the strictQualityIndicator option!
     */
    QualityEstimatorMC(const std::string& mcRecoTracksStoreArrayName = "MCRecoTracks",
                       bool strictQualityIndicator = true, bool mva_target = false):
      QualityEstimatorBase(), m_strictQualityIndicator(strictQualityIndicator), m_mva_target(mva_target),
      m_mcRecoTracksStoreArrayName(mcRecoTracksStoreArrayName)
    {
      m_mcRecoTracks.isRequired(m_mcRecoTracksStoreArrayName);
    };

    /** Performing MC based quality estimation
     * @param measurements : SPs of the track candidate to be evaluate
     * @return quality indicator value
     */
    virtual double estimateQuality(std::vector<SpacePoint const*> const& measurements) final;

    /** additionally return momentum_truth if it is a perfect match to a single MCRecoTrack */
    virtual QualityEstimationResults estimateQualityAndProperties(std::vector<SpacePoint const*> const& measurements) override final;

    /** Setter for StoreArray names of SVD and PXD and VTX clusters
     * @param svdClustersName : SVD cluster StoreArray name
     * @param pxdClustersName : PXD cluster StoreArray name
     * @param vtxClustersName : VTX cluster StoreArray name
     */
    void setClustersNames(const std::string& svdClustersName, const std::string& pxdClustersName, const std::string& vtxClustersName)
    { m_svdClustersName = svdClustersName; m_pxdClustersName = pxdClustersName;  m_vtxClustersName = vtxClustersName;};

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
    double calculateQualityIndicator(unsigned int nClusters, MatchInfo& match);

    // parameters
    /** If true only SPTCs containing SVDClusters corresponding to a single MCRecoTrack get a QI != 0.
     *  If a SVDCluster corresponds to several MCRecoTracks it is still valid as long as the correct MCRecoTrack is one of them.
     *  FIXME: this should be adjuested for VTX
     */
    bool m_strictQualityIndicator;

    /** If true the SPTCs containing all the SVDCluster of the corresponding MCRecoTrack and no other SVDCluster
     *  receive a Quality Index larger than one. To be used for MVA QE training if both high hit purity and hit
     *  efficiency is desired.
     *  FIXME: this should be adjuested for VTX
     */
    bool m_mva_target;

    // module members
    std::string m_mcRecoTracksStoreArrayName; /**< MCRecoTracks StoreArray name */
    std::string m_svdClustersName = ""; /**< SVD clusters StoreArray name */
    std::string m_pxdClustersName = ""; /**< PXD clusters StoreArray name */
    std::string m_vtxClustersName = ""; /**< VTX clusters StoreArray name */

    /** stores the current match for optional return values */
    MatchInfo m_match;

    /** the storeArray for RecoTracks as member */
    StoreArray<RecoTrack> m_mcRecoTracks;
  };
}
