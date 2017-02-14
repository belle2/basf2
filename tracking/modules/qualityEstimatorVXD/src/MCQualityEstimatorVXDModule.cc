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


  addParam("SPTCsName", m_SPTCsName, "Sets the name of expected StoreArray with SpacePointTrackCand in it", std::string(""));

  addParam("mcRecoTracksName", m_mcRecoTracksName, "Sets the name of expected StoreArray with mcRecoTracks in it",
           std::string("MCRecoTracks"));

  addParam("strictQualityIndex", m_strictQualityIndex,
           "If true only SPTCs containing SVDClusters corresponding to a single MCRecoTrack get a QI != 0.", true);
}

void MCQualityEstimatorVXDModule::event()
{

  if (m_mcRecoTracks.getEntries() == 0) {
    B2WARNING("MCQualityEstimator::Event: No MCRecoTracks present. Skipping this module.");
    return;
  }
  // store to make sure SPTCs are compared to the correct SVDStoreArray
  m_svdClustersName = m_mcRecoTracks[0]->getStoreArrayNameOfSVDHits();

  // initialize Matrix
  m_linkMatrix = Eigen::SparseMatrix<bool> (m_mcRecoTracks.getEntries(), StoreArray<SVDCluster>("").getEntries());
  m_linkMatrix.reserve(m_mcRecoTracks.getEntries() * 8);

  // create relation SVDClusterIndex to MCRecoTrackIndex
  fillMatrixWithMCInformation();

  // evaluate Matrix for each SPTC
  for (SpacePointTrackCand& spTC : m_spacePointTrackCands) {

    std::pair<int, int> match = getBestMatchToMCClusters(spTC);

    // assuming that each SpacePoint corresponds to two Clusters. Only valid for SVD!
    double qi = calculateQualityIndex(spTC.getNHits() * 2, match);
    spTC.setQualityIndex(qi);
  } // end loop SpacePointTrackCandidates
}

void MCQualityEstimatorVXDModule::fillMatrixWithMCInformation()
{
  // fill Matrix
  for (RecoTrack& mcRecoTrack : m_mcRecoTracks) {
    int mcRecoTrackIndex = mcRecoTrack.getArrayIndex();
    for (SVDCluster* cluster : mcRecoTrack.getSVDHitList()) {
      m_linkMatrix.insert(mcRecoTrackIndex, cluster->getArrayIndex()) = true;
    }
  }
  B2DEBUG(1, "Number of Entries: " << m_linkMatrix.nonZeros() << ", OuterSize: " << m_linkMatrix.outerSize() << ", InnerSize: " <<
          m_linkMatrix.innerSize());
}

std::pair<int, int> MCQualityEstimatorVXDModule::getBestMatchToMCClusters(SpacePointTrackCand sptc)
{
  std::map<int, int> matches;

  for (const SpacePoint* spacePoint : sptc.getHits()) {
    for (SVDCluster& cluster : spacePoint->getRelationsTo<SVDCluster>(m_svdClustersName)) {
      int svdClusterIndex = cluster.getArrayIndex();

      // Due to MCRecoTracks overlapping each SVDCluster might match to multiple MCRecoTracks
      for (Eigen::SparseMatrix<bool>::InnerIterator it(m_linkMatrix, svdClusterIndex); it; ++it) {
        ++matches[it.row()];
      }

    } // end loop SVDClusters
  } // end loop SpacePoints

  // select best match as the one with the most matched clusters.
  std::pair<int, int> bestMatch(0, 0);
  for (auto& relation : matches) {
    if (relation.second > bestMatch.second) bestMatch = relation;
  }
  return bestMatch;
}

double MCQualityEstimatorVXDModule::calculateQualityIndex(int nClusters, std::pair<int, int> match)
{
  double qualityIndex = 0;
  if (m_strictQualityIndex) {
    if (nClusters == match.second) qualityIndex = 1 - (1. / nClusters);
  } else {
    int nRecoTrackClusters =  m_mcRecoTracks[match.first]->getNumberOfSVDHits();
    qualityIndex = std::pow(match.second, 3) / (nRecoTrackClusters * nClusters * nClusters);
  }
  return qualityIndex;
}
