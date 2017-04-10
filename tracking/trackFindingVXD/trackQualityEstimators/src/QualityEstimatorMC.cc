/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Metzner, Jonas Wagner                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h"
#include <mdst/dataobjects/MCParticle.h>
#include <math.h>

using namespace Belle2;

void QualityEstimatorMC::fillMatrixWithMCInformation()
{
  // fill Matrix
  for (RecoTrack& mcRecoTrack : m_mcRecoTracks) {
    int mcRecoTrackIndex = mcRecoTrack.getArrayIndex();
    for (SVDCluster* cluster : mcRecoTrack.getSVDHitList()) {
      m_linkMatrix.insert(mcRecoTrackIndex, cluster->getArrayIndex()) = true;
    }
  }
}

double QualityEstimatorMC::estimateQuality(std::vector<SpacePoint const*> const& measurements)
{
  // initialize Matrix
  m_linkMatrix = Eigen::SparseMatrix<bool> (m_mcRecoTracks.getEntries(), StoreArray<SVDCluster>("").getEntries());
  // Best guess about required size. Average should be slightly below 8 SVD clusters (2 per layer) per Track.
  m_linkMatrix.reserve(m_mcRecoTracks.getEntries() * 8);
  // create relation SVDClusterIndex to MCRecoTrackIndex
  fillMatrixWithMCInformation();

  m_match = getBestMatchToMCClusters(measurements);

  // assuming that each SpacePoint corresponds to two Clusters. Only valid for SVD!
  return calculateQualityIndex(measurements.size() * 2, m_match);
}

QualityEstimatorMC::MatchInfo QualityEstimatorMC::getBestMatchToMCClusters(std::vector<SpacePoint const*> const& measurements)
{
  std::map<MCRecoTrackIndex, NMatches> matches;

  for (SpacePoint const* spacePoint : measurements) {
    for (SVDCluster& cluster : spacePoint->getRelationsTo<SVDCluster>(m_svdClustersName)) {
      int svdClusterIndex = cluster.getArrayIndex();

      // Due to MCRecoTracks overlapping each SVDCluster might match to multiple MCRecoTracks
      for (Eigen::SparseMatrix<bool>::InnerIterator it(m_linkMatrix, svdClusterIndex); it; ++it) {
        MCRecoTrackIndex index = it.row();
        // Increase number of matches to this RecoTrack
        ++matches[index];
      }

    } // end loop SVDClusters
  } // end loop SpacePoints

  // select best match as the one with the most matched clusters.
  MatchInfo bestMatch = *std::max_element(matches.begin(), matches.end(),
  [](MatchInfo const & lhs, MatchInfo const & rhs) {return lhs.second < rhs.second;});
  return bestMatch;
}

double QualityEstimatorMC::calculateQualityIndex(int nClusters, MatchInfo& match)
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


QualityEstimationResults QualityEstimatorMC::estimateQualityAndProperties(std::vector<SpacePoint const*> const& measurements)
{
  QualityEstimatorBase::estimateQualityAndProperties(measurements);
  if (m_results.qualityIndicator != 0) {
    auto mcParticle = m_mcRecoTracks[m_match.first]->getRelated<MCParticle>();
    m_results.p = mcParticle->getMomentum();
    m_results.pt = mcParticle->get4Vector().Pt();
  }
  return m_results;
}


