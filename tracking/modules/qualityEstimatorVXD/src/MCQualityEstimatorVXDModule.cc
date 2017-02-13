/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/qualityEstimatorVXD/MCQualityEstimatorVXDModule.h>
#include <framework/logging/Logger.h>
#include <exception>

using namespace Belle2;

REG_MODULE(MCQualityEstimatorVXD)


MCQualityEstimatorVXDModule::MCQualityEstimatorVXDModule() : Module()
{
  setDescription("The quality estimator module for SpacePointTrackCandidates using a MC information.");
  setPropertyFlags(c_ParallelProcessingCertified);


  addParam("SPTCsName", m_SPTCsName, " Sets the name of expected StoreArray with SpacePointTrackCand in it", std::string(""));

  addParam("mcRecoTracksName", m_mcRecoTracksName, " Sets the name of expected StoreArray with mcRecoTracks in it",
           std::string("MCRecoTracks"));

}

void MCQualityEstimatorVXDModule::event()
{
  // create relation SVDClusterID to MCRecoTrackID
  fillSVDClusterToRecoTrackMap();




  for (SpacePointTrackCand& spTC : m_spacePointTrackCands) {
    // count matches
    int nMatches = countMatchesToMCRecoTrack(spTC);

    spTC.setQualityIndex(1 - (1. / nMatches));
  } // end loop SpacePointTrackCandidates
}

void MCQualityEstimatorVXDModule::fillSVDClusterToRecoTrackMap()
{
  m_mapIndexSVDToIndexRecoTrack.clear();
  for (RecoTrack& mcRecoTrack : m_mcRecoTracks) {
    int mcRecoTrackID = mcRecoTrack.getArrayIndex();
    for (SVDCluster* cluster : mcRecoTrack.getSVDHitList()) {
      m_mapIndexSVDToIndexRecoTrack[cluster->getArrayIndex()] = mcRecoTrackID;
    }
  }
}

int MCQualityEstimatorVXDModule::countMatchesToMCRecoTrack(SpacePointTrackCand sptc)
{
  int relatedRecoTrackIndex = -1;

  for (const SpacePoint* spacePoint : sptc.getHits()) {
    for (SVDCluster& cluster : spacePoint->getRelationsTo<SVDCluster>()) {
      // find related RecoTrack
      auto iter = m_mapIndexSVDToIndexRecoTrack.find(cluster.getArrayIndex());
      if (iter == m_mapIndexSVDToIndexRecoTrack.end()) return 0;

      // if it is the first relation store value
      if (relatedRecoTrackIndex == -1) relatedRecoTrackIndex = iter->second;

      // verify it is the same RecoTrack as before
      if (relatedRecoTrackIndex != iter->second) return 0;
    }
  } // end loop SpacePoints
  return sptc.getNHits();
}
