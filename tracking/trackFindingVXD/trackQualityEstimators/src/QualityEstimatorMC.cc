/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h"
#include <mdst/dataobjects/MCParticle.h>

using namespace Belle2;

double QualityEstimatorMC::estimateQuality(std::vector<SpacePoint const*> const& measurements)
{
  if (measurements.empty()) return 0;

  m_match = getBestMatchToMCClusters(measurements);

  // assuming that each SpacePoint corresponds to two Clusters. Now valid for both SVD and PXD
  return calculateQualityIndicator(measurements.size() * 2, m_match);
}

QualityEstimatorMC::MatchInfo QualityEstimatorMC::getBestMatchToMCClusters(std::vector<SpacePoint const*> const& measurements)
{
  std::map<MCRecoTrackIndex, NMatches> matches;

  for (SpacePoint const* spacePoint : measurements) {

    for (SVDCluster& cluster : spacePoint->getRelationsTo<SVDCluster>(m_svdClustersName)) {
      for (RecoTrack& recoTrack : cluster.getRelationsWith<RecoTrack>(m_mcRecoTracksStoreArrayName)) {
        // Increase number of matches to this RecoTrack
        matches[recoTrack.getArrayIndex()]++;
      }
    } // end loop SVDClusters

    for (PXDCluster& cluster : spacePoint->getRelationsTo<PXDCluster>(m_pxdClustersName)) {
      for (RecoTrack& recoTrack : cluster.getRelationsWith<RecoTrack>(m_mcRecoTracksStoreArrayName)) {
        // Increase number of matches to this RecoTrack
        matches[recoTrack.getArrayIndex()] += 2;
      }
    } // end loop PXDClusters

    for (VTXCluster& cluster : spacePoint->getRelationsTo<VTXCluster>(m_vtxClustersName)) {
      for (RecoTrack& recoTrack : cluster.getRelationsWith<RecoTrack>(m_mcRecoTracksStoreArrayName)) {
        // Increase number of matches to this RecoTrack
        matches[recoTrack.getArrayIndex()] += 2;
      }
    } // end loop VTXClusters

  } // end loop SpacePoints

  // select best match as the one with the most matched clusters.
  MatchInfo bestMatch = *std::max_element(matches.begin(), matches.end(),
  [](MatchInfo const & lhs, MatchInfo const & rhs) {return lhs.second < rhs.second;});
  return bestMatch;
}

double QualityEstimatorMC::calculateQualityIndicator(unsigned int nClusters, MatchInfo& match)
{
  double qualityIndicator = 0;
  if (m_mva_target) { // FIXME: this should be adjuested for VTX
    if (nClusters == m_mcRecoTracks[match.first]->getNumberOfSVDHits()) {
      qualityIndicator = 1 - (1. / nClusters);
    }
  } else if (m_strictQualityIndicator) {
    if (nClusters == match.second) {
      qualityIndicator = 1 - (1. / nClusters);
    }
  } else {  // FIXME: this should be adjuested for VTX
    int nRecoTrackClusters =  m_mcRecoTracks[match.first]->getNumberOfSVDHits();
    qualityIndicator = std::pow(match.second, 3) / (nRecoTrackClusters * nClusters * nClusters);
  }
  return qualityIndicator;
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
