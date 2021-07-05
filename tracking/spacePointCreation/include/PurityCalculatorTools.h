/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
#include <cmath> // for fabs

namespace Belle2 { // make seperate sub-namespace for this?

  /**
   * find the given weight in the given vector of pair<int,double>
   * NOTE: the criteria for finding are rather loose (i.e. actual value and desired value must not differ by more than 1e-4) as
   * the values that are encountered lie rather far apart!
   */
  static bool findWeightInVector(std::vector<std::pair<int, double> >& vec, double weight)
  {
    const double epsilon = 1e-4; // no real need for accuracy here since weights are rather far apart!
    return (std::find_if(vec.begin(), vec.end(),
                         [&epsilon, &weight](const std::pair<int, double>& p)
    { return fabs(p.second - weight) <= epsilon; })
    != vec.end());
  }

  /**
   * get the related MCParticles to the TrueHit.
   * @returns vector of pairs, where .first is the MCParticleId, which is -1 if there was no MCParticle related to a TrueHit
   * and -2 if there is a Cluster without a relation to a TrueHit (probably a noise Cluster then)
   * .second is the relation weight between SpacePoint and TrueHit (encodes additional information that is needed afterwards)
   */
  template<typename TrueHitType>
  static std::vector<std::pair<int, double> > getMCParticles(const Belle2::SpacePoint* spacePoint)
  {
    // CAUTION: getting all TrueHits here (i.e. from all StoreArrays)
    Belle2::RelationVector<TrueHitType> trueHits = spacePoint->getRelationsTo<TrueHitType>("ALL");
    std::vector<std::pair<int, double> > mcParticles;
    B2DEBUG(4999, "Found " << trueHits.size() << " TrueHits related to SpacePoint " << spacePoint->getArrayIndex());

    for (size_t iTH = 0; iTH < trueHits.size(); ++iTH) {
      B2DEBUG(4999, "Trying to get MCParticles from TrueHit " << trueHits[iTH]->getArrayIndex() <<
              " from Array " << trueHits[iTH]->getArrayName());
      Belle2::MCParticle* mcPart = trueHits[iTH]->template getRelatedFrom<Belle2::MCParticle>("ALL");

      int mcPartId = -1; // default value for MCPartId (not found)
      if (mcPart != nullptr) {
        mcPartId = mcPart->getArrayIndex();
        B2DEBUG(4999, "TrueHit is related to MCParticle " << mcPartId);
      } else {
        B2DEBUG(1, "Found no MCParticle related to TrueHit " << trueHits[iTH]->getArrayIndex() <<
                " from Array " << trueHits[iTH]->getArrayName());
      }
      mcParticles.push_back(std::make_pair(mcPartId, trueHits.weight(iTH)));
    }

    // sort & unique to filter out double MCIds (if the relations are set correct, this only happens if more than one TrueHits point to the same MCParticle)
    std::sort(mcParticles.begin(), mcParticles.end());
    auto newEnd = std::unique(mcParticles.begin(), mcParticles.end(),
    [](const std::pair<int, double>& a, const std::pair<int, double>& b) { return a.first == b.first; }
                             );
    if (newEnd != mcParticles.end()) B2DEBUG(1,
                                               "More than one TrueHits (related to one SpacePoint) are related to the same MCParticle!");
    mcParticles.resize(std::distance(mcParticles.begin(), newEnd));

    // check if there were noise Clusters in the SpacePoint
    // only case left for PXD is missing TrueHit for underlying Cluster, everything else is covered!
    if (spacePoint->getType() == VXD::SensorInfoBase::PXD) {
      if (mcParticles.empty()) mcParticles.push_back(std::make_pair(-2, 1)); // if no Id in vector there is no relation to TrueHit
    } else  if (spacePoint->getType() == VXD::SensorInfoBase::SVD) { // for SVD some more tests are needed!
      // check if there is one MCParticle that is related to both Clusters of the SpacePoint
      if (!findWeightInVector(mcParticles, 2)) { // if not both related check which ones should have a relation and act accordingly
        const std::pair<bool, bool>& clusters = spacePoint->getIfClustersAssigned();
        bool uGood = clusters.first && findWeightInVector(mcParticles, 11);
        bool vGood = clusters.second && findWeightInVector(mcParticles, 21);
        if (!uGood && !vGood) mcParticles.push_back(std::make_pair(-2, 2)); // if both Clusters are not related -> weight is 2
        else if (uGood && !vGood) mcParticles.push_back(std::make_pair(-2, 21)); // if only V-Cluster is not related -> weight is 21
        else if (!uGood && vGood) mcParticles.push_back(std::make_pair(-2, 11)); // if only U-Cluster is not related -> weight is 11
      }
    } else {
      B2ERROR("Unknown DetectorType in getMCParticles! This is just a notification! Needs to be handled!"); // should not happen
    }

    return mcParticles;
  }

  /**
   * increase the appropriate Cluster counter by asking the SpacePoint which type he has and which Clusters are assigned
   */
  static void increaseClusterCounters(const Belle2::SpacePoint* spacePoint, std::array<unsigned, 3>& clusterCtr)
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
  static std::vector<size_t> getAccessorsFromWeight(double weight)
  {
    if (weight < 1.5 && weight > 0) return {0}; // weight 1 -> PXD
    if (weight < 2.5 && weight > 0) return { 1, 2 }; // weight 2 -> both Clusters with relation to MCParticle
    if (weight < 20 && weight > 0) return {1}; // weight 11 -> only U-Cluster related to MCParticle
    if (weight > 20 && weight < 30) return {2}; // weight 21 -> only V-Cluster related to MCParticle

    return std::vector<size_t>(); // empty vector else
  }

  /** create a vector of MCVXDPurityInfos objects for a std::vector<Belle2::SpacePoints>.
   * each MCParticle that is in the vector gets its own object
   * NOTE: negative MCParticleIds are possible and used as follows:
   * + -1 -> there was a TrueHit (i.e. Cluster) related to a SpacePoint in the vector that did not have a relation to a MCParticle
   * + -2 -> there was a SpacePoint with a Cluster that was not related to a TrueHit (noise Cluster)
   * @returns sorted vector of MCVXDPurityInfo (sorted by overall purity)
   */
  static std::vector<Belle2::MCVXDPurityInfo> createPurityInfosVec(const std::vector<const Belle2::SpacePoint*>& spacePoints)
  {
    std::array<unsigned, 3> totalClusters = { {0, 0, 0 } };
    // WARNING: relying on the fact here that all array entries will be initialized to 0
    std::unordered_map<int, std::array<unsigned, 3> > mcClusters;

    unsigned nClusters = 0; // NOTE: only needed for DEBUG output -> remove?

    for (const Belle2::SpacePoint* spacePoint : spacePoints) {
      B2DEBUG(4999, "Now processing SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());
      increaseClusterCounters(spacePoint, totalClusters);

      nClusters += spacePoint->getNClustersAssigned();

      std::vector<std::pair<int, double> > mcParticles;
      if (spacePoint->getType() == VXD::SensorInfoBase::PXD) mcParticles = getMCParticles<PXDTrueHit>(spacePoint);
      else if (spacePoint->getType() == VXD::SensorInfoBase::SVD) mcParticles = getMCParticles<SVDTrueHit>(spacePoint);
      else if (spacePoint->getType() == VXD::SensorInfoBase::VXD) {B2DEBUG(100, "found generic spacePoint, treating it as virtualIP");}
      else B2FATAL("Unknown DetectorType (" << spacePoint->getType() << ") in createPurityInfos! Skipping this SpacePoint " <<
                     spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());

      for (const std::pair<int, double>& particle : mcParticles) {
        std::vector<size_t> accessors = getAccessorsFromWeight(particle.second);
        for (size_t acc : accessors) {
          mcClusters[particle.first][acc]++;
        }
      }
    }
    B2DEBUG(4999, "container contained " << spacePoints.size() << " SpacePoint with " << nClusters << " Clusters");

    // create MCVXDPurityInfos and add them to the return vector
    std::vector<Belle2::MCVXDPurityInfo> purityInfos;
    for (int mcId : getUniqueKeys(mcClusters)) {
      // cppcheck-suppress useStlAlgorithm
      purityInfos.push_back(MCVXDPurityInfo(mcId, totalClusters, mcClusters[mcId]));
    }

    // sort in descending order before return
    std::sort(purityInfos.begin(), purityInfos.end(),
    [](const MCVXDPurityInfo & left, const MCVXDPurityInfo & right) { return left > right; }
             );
    return purityInfos;
  }

  /**
   * create a vector of MCVXDPurityInfos objects for any given container holding SpacePoints and providing a getHits() method
   * each MCParticle that is in the container gets its own object
   * NOTE: negative MCParticleIds are possible und used as follows:
   * + -1 -> there was a TrueHit (i.e. Cluster) related to a SpacePoint in the container that did not have a relation to a MCParticle
   * + -2 -> there was a SpacePoint with a Cluster that was not related to a TrueHit (noise Cluster)
   * @returns sorted vector of MCVXDPurityInfo (sorted by overall purity)
   */
  template<typename SPContainer>
  static std::vector<Belle2::MCVXDPurityInfo> createPurityInfos(const SPContainer* container)
  {
    return createPurityInfosVec(container->getHits());
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
  static std::vector<Belle2::MCVXDPurityInfo> createPurityInfos(const SPContainer& container)
  {
    return createPurityInfos(&container);
  }
} // end namespace Belle2
