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

#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/core/Module.h>
#include <Eigen/Sparse>


namespace Belle2 {

  /** The quality estimator module for SpacePointTrackCandidates using MC information.
   * WARNING: Only working for SVDClusters!
   * */
  class MCQualityEstimatorVXDModule : public Module {

  public:


    /** Constructor of the module. */
    MCQualityEstimatorVXDModule();

    void initialize()
    {
      m_spacePointTrackCands.isRequired(m_SPTCsName);
      m_mcRecoTracks.isRequired(m_mcRecoTracksName);
    }

    void event();


  protected:

    // some typedefs to increase readability
    typedef int MCRecoTrackIndex;
    typedef int NMatches;
    typedef std::pair<MCRecoTrackIndex, NMatches> MatchInfo;

    void fillMatrixWithMCInformation();

    MatchInfo getBestMatchToMCClusters(SpacePointTrackCand& sptc);

    double calculateQualityIndex(int nClusters, MatchInfo& match);


    // module parameters

    /** sets the name of expected StoreArray with mcRecoTracks in it. */
    std::string m_mcRecoTracksName;

    /** sets the name of expected StoreArray with SPTCs in it. */
    std::string m_SPTCsName;

    /** If true only SPTCs containing SVDClusters corresponding to a single MCRecoTrack get a QI != 0.
     *  If a SVDCluster corresponds to several MCRecoTracks it is still valid as long as the correct MCRecoTrack is one of them.
     */
    bool m_strictQualityIndex;

    // module members

    std::string m_svdClustersName;

    /** the storeArray for SpacePointTrackCands as member */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** the storeArray for RecoTracks as member */
    StoreArray<RecoTrack> m_mcRecoTracks;

    /** store relation SVDClusterIndex to MCRecoTrackIndex */
    Eigen::SparseMatrix<bool> m_linkMatrix;
  };
}
