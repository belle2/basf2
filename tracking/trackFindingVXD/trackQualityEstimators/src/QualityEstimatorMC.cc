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
#include <framework/logging/Logger.h>

using namespace Belle2;

double QualityEstimatorMC::estimateQuality(std::vector<constSPpointer> const& measurements)
{
  if (measurements.empty()) return 0;

  m_match = getBestMatchToMCClusters(measurements);

  int n_svd_spacepoints = 0; int n_pxd_spacepoints = 0;

  for (auto& spacepoint : measurements) {
    if (spacepoint->getType() == VXD::SensorInfoBase::SVD)
      n_svd_spacepoints++;
    else if (spacepoint->getType() == VXD::SensorInfoBase::PXD)
      n_pxd_spacepoints++;
    else
      B2FATAL("QualityEstimatorMC: SpacePoint in measurements vector has a Type != SVD or PXD");
  }

  return calculateQualityIndex(n_svd_spacepoints * 2 + n_pxd_spacepoints, m_match);
}

QualityEstimatorMC::MatchInfo QualityEstimatorMC::getBestMatchToMCClusters(std::vector<constSPpointer> const& measurements)
{
  std::map<MCRecoTrackIndex, NMatches> matches;

  for (constSPpointer spacePoint : measurements) {

    for (SVDCluster& cluster : spacePoint->getRelationsTo<SVDCluster>(m_svdClustersName)) {
      for (RecoTrack& recoTrack : cluster.getRelationsWith<RecoTrack>(m_mcRecoTracksStoreArrayName)) {
        // Increase number of matches to this RecoTrack
        ++matches[recoTrack.getArrayIndex()];
      }
    } // end loop SVDClusters

    for (PXDCluster& cluster : spacePoint->getRelationsTo<PXDCluster>(m_pxdClustersName)) {
      for (RecoTrack& recoTrack : cluster.getRelationsWith<RecoTrack>(m_mcRecoTracksStoreArrayName)) {
        // Increase number of matches to this RecoTrack
        ++matches[recoTrack.getArrayIndex()];
      }
    } // end loop PXDClusters

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


QualityEstimationResults QualityEstimatorMC::estimateQualityAndProperties(std::vector<constSPpointer> const& measurements)
{
  QualityEstimatorBase::estimateQualityAndProperties(measurements);
  if (m_results.qualityIndicator != 0) {
    auto mcParticle = m_mcRecoTracks[m_match.first]->getRelated<MCParticle>();
    m_results.p = mcParticle->getMomentum();
    m_results.pt = mcParticle->get4Vector().Pt();
  }
  return m_results;
}
