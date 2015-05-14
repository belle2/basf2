/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <tracking/spacePointCreation/MCVXDPurityInfo.h>

#include <framework/datastore/RelationVector.h>
#include <mdst/dataobjects/MCParticle.h>

#include <tracking/spacePointCreation/SpacePoint.h>
//#include <vxd/dataobjects/VXDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <tracking/spacePointCreation/MapHelperFunctions.h>

#include <vector>
#include <utility> // for pair
#include <unordered_map>
#include <cmath> // for nearbyint

namespace Belle2 {

  /**
   * get the related MCParticles to the TrueHit.
   * @returns vector of pairs, where .first is the MCParticleId, which is -1 if there was no MCParticle related to a TrueHit.
   * .second is the relation weight between SpacePoint and TrueHit (encodes additional information that is needed afterwards)
   */
  template<typename TrueHitType>
  std::vector<std::pair<int, double> > getMCParticles(const Belle2::SpacePoint* spacePoint)
  {
    // CAUTION: getting all TrueHits here
    Belle2::RelationVector<TrueHitType> trueHits = spacePoint->getRelationsTo<TrueHitType>("ALL");
    std::vector<std::pair<int, double> > mcParticles;

    for (size_t iTH = 0; iTH < trueHits.size(); ++iTH) {
      B2DEBUG(4999, "Trying to get MCParticles from TrueHit " << trueHits[iTH]->getArrayIndex() <<
              " from Array " << trueHits[iTH]->getArrayName());
      Belle2::MCParticle* mcPart = trueHits[iTH]->template getRelatedFrom<Belle2::MCParticle>("ALL");

      int mcPartId = -1; // default value for MCPartId (not found)
      if (mcPart != NULL) {
        mcPartId = mcPart->getArrayIndex();
        B2DEBUG(4999, "TrueHit is related to MCParticle " << mcPartId);
      } else {
        B2WARNING("Found no MCParticle related to TrueHit " << trueHits[iTH]->getArrayIndex() <<
                  " from Array " << trueHits[iTH]->getArrayName());
      }
      mcParticles.push_back(std::make_pair(mcPartId, trueHits.weight(iTH)));
    }

    // sort & unique to filter out double MCIds (if the relations are set correct, this only happens if more than one TrueHits point to the same MCParticle)
    std::sort(mcParticles.begin(), mcParticles.end());
    auto newEnd = std::unique(mcParticles.begin(), mcParticles.end(),
    [](const std::pair<int, double>& a, const std::pair<int, double>& b) { return a.first == b.first; }
                             );
    if (newEnd != mcParticles.end()) B2ERROR("More than one TrueHits (related to one SpacePoint) are related to the same MCParticle!");
    mcParticles.resize(std::distance(mcParticles.begin(), newEnd));

    return mcParticles;
  }

  /**
   * increase the appropriate Cluster counter by asking the SpacePoint which type he has and which Clusters are assigned
   */
  void increaseClusterCounters(const Belle2::SpacePoint* spacePoint, std::array<unsigned, 3>& clusterCtr)
  {
    if (spacePoint->getType() == Belle2::VXD::SensorInfoBase::PXD) {
      clusterCtr[0]++;
      B2DEBUG(4999, "SpacePoint contains PXDCluster");
    } else {
      std::pair<bool, bool> setClusters = spacePoint->getIfClustersAssigned();
      if (setClusters.first) {
        clusterCtr[1]++;
        B2DEBUG(4999, "SpacePoint contains U SVDCluster");
      }
      if (setClusters.second) {
        clusterCtr[2]++;
        B2DEBUG(4999, "SpacePoint contains V SVDCluster");
      }
    }
  }

  /**
   * convert the relation weight (SpacePoint <-> TrueHit) to a type that can be used to access arrays
   */
  std::vector<size_t> getAccessorsFromWeight(double weight)
  {
    if (weight < 1.5 && weight > 0) return {0}; // weight 1 -> PXD
    if (weight < 2.5) return { 1, 2 }; // weight 2 -> both Clusters with relation to MCParticle
    if (weight < 20) return {1}; // weight 11 -> only U-Cluster related to MCParticle
    if (weight > 20 && weight < 30) return {2}; // weight 21 -> only V-Cluster related to MCParticle

    return std::vector<size_t>(); // empty vector else
  }

  /**
   * create a vector of MCVXDPurityInfos objects for any given container holding SpacePoints and providing a getHits() method
   * each MCParticle that is in the container gets its own object
   * NOTE: negative MCParticleIds are possible und used as follows:
   * + -1 -> there was a TrueHit (i.e. Cluster) related to a SpacePoint in the container that did not have relation to a MCParticle
   * + -2 -> there was a SpacePoint with a Cluster that was not related to a TrueHit (noise Cluster)
   * @returns sorted vector of MCVXDPurityInfo (sorted by overall purity)
   */
  template<typename SPContainer>
  std::vector<Belle2::MCVXDPurityInfo> createPurityInfos(const SPContainer* container)
  {
    std::array<unsigned, 3> totalClusters = { {0, 0, 0 } };
    // WARNING: relying on the fact here that all array entries will be initialized to 0
    std::unordered_map<int, std::array<unsigned, 3> > mcClusters;

    unsigned nClusters = 0;

    // collect all information
    for (const Belle2::SpacePoint* spacePoint : container->getHits()) {
      B2DEBUG(4999, "Now processing SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());
      increaseClusterCounters(spacePoint, totalClusters);

      nClusters += spacePoint->getNClustersAssigned();

      std::vector<std::pair<int, double> > mcParticles;
      if (spacePoint->getType() == VXD::SensorInfoBase::PXD) mcParticles = getMCParticles<PXDTrueHit>(spacePoint);
      else if (spacePoint->getType() == VXD::SensorInfoBase::SVD) mcParticles = getMCParticles<SVDTrueHit>(spacePoint);
      else B2ERROR("Unknown DetectorType in createPurityInfos! Skipping this SpacePoint");

      for (const std::pair<int, double>& particle : mcParticles) {
        std::vector<size_t> accessors = getAccessorsFromWeight(particle.second);
        for (size_t acc : accessors) {
          mcClusters[particle.first][acc]++;
        }
      }
    }

    B2DEBUG(4999, "container contained " << container->getNHits() << " SpacePoint with " << nClusters << " Clusters");
    // TODO: handle noise Clusters

    // create MCVXDPurityInfos and add them to the return vector
    std::vector<Belle2::MCVXDPurityInfo> purityInfos;
    for (int mcId : getUniqueKeys(mcClusters)) {
      purityInfos.push_back(MCVXDPurityInfo(mcId, totalClusters, mcClusters[mcId]));
    }

    std::sort(purityInfos.begin(), purityInfos.end());
    return purityInfos;
  }

  /**
   * * create a vector of MCVXDPurityInfos objects for any given container holding SpacePoints and providing a getHits() method
   * each MCParticle that is in the container gets its own object
   * NOTE: negative MCParticleIds are possible und used as follows:
   * + -1 -> there was a TrueHit (i.e. Cluster) related to a SpacePoint in the container that did not have relation to a MCParticle
   * + -2 -> there was a SpacePoint with a Cluster that was not related to a TrueHit (noise Cluster)
   * @returns sorted vector of MCVXDPurityInfo (sorted by overall purity)
   *
   * wrapper taking a const reference instead of a const pointer for use in e.g. C++11 for loops
   */
  template<typename SPContainer>
  std::vector<Belle2::MCVXDPurityInfo> createPurityInfos(const SPContainer& container)
  {
    return createPurityInfos(&container);
  }
} // end namespace

