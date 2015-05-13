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

#include <vector>
#include <utility> // for pair
#include <unordered_map>

#include <framework/datastore/RelationVector.h>
#include <mdst/dataobjects/MCParticle.h>

#include <tracking/spacePointCreation/SpacePoint.h>
//#include <vxd/dataobjects/VXDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <tracking/spacePointCreation/MapHelperFunctions.h>

namespace Belle2 {

  /**
   * get the related MCParticles and the number of Clusters they are related to for a SpacePoint
   * + If there is a TrueHit with no relation to a MCParticle, the Cluster is consiedered to be related to MCId -1 (CAUTION: cannot
   * be accesed in StoreArray!)
   *
   * CAUTION: The method cannot check if there is a TrueHit to a Cluster, hence it is possible that it returns an empty vector or a
   * vector that is filled with MCParticleIds that are only related to one Cluster of the SpacePoint!
   * This issue has to be adressed during the creation of the Relations between SpacePoints and TrueHits! -> TODO!!!
   */
  template<typename TrueHitType>
  std::vector<std::pair<int, short> > getMCParticles(const Belle2::SpacePoint* spacePoint)
  {
    // CAUTION: getting all TrueHits here! if needed restrict this!
    Belle2::RelationVector<TrueHitType> trueHits = spacePoint->getRelationsTo<TrueHitType>("ALL");
    std::vector<std::pair<int, short> > mcParticles;

    for (size_t iTH = 0; iTH < trueHits.size(); ++iTH) {
      B2DEBUG(999, "Trying to get MCParticles from TrueHit " << trueHits[iTH]->getArrayIndex() <<
              " from Array " << trueHits[iTH]->getArrayName());
      Belle2::MCParticle* mcPart = trueHits[iTH]->template getRelatedFrom<Belle2::MCParticle>("ALL");

      int mcPartId = -1; // default value for MCPartId (not found)
      if (mcPart != NULL) {
        mcPartId = mcPart->getArrayIndex();
        B2DEBUG(999, "TrueHit " << iTH << " MCPartId " << mcPartId);
      } else {
        B2WARNING("Found no MCParticle related to TrueHit " << trueHits[iTH]->getArrayIndex() <<
                  " from Array " << trueHits[iTH]->getArrayName());
      }
      mcParticles.push_back(std::make_pair(mcPartId, trueHits.weight(iTH)));
    }

    return mcParticles;
  }

  /**
   * function that takes any container holding SpacePoints that provide a getHits() function.
   * @return vector of pairs where .first is an McId and .second is the purity of this McId
   *
   * The purities are calculated via (# Clusters associated with MCId) / (# Clusters in all SpacePoints of container)
   * Clusters that cannot be associated with a MCParticle are put into MCId -1
   */
  template<typename SPContainer>
  std::vector<std::pair<int, double> > calculatePurity(const SPContainer* container)
  {
    std::vector<std::pair<int, double> > purities;
    const std::vector<const Belle2::SpacePoint*>& spacePoints = container->getHits();

    // store the MCParticle Ids and the number of related Clusters to that id into a map (use mapHelperFunctions from there on)
    std::unordered_map<int, unsigned> mcPartMap;
    unsigned nClusters = 0;

    for (const Belle2::SpacePoint* spacePoint : spacePoints) {
      B2DEBUG(999, "Getting MCParticles from SpacePoint " << spacePoint->getArrayIndex() << " from Array " <<
              spacePoint->getArrayName());
      std::vector<std::pair<int, short> > mcParticles;

      if (spacePoint->getType() == VXD::SensorInfoBase::PXD) mcParticles = getMCParticles<PXDTrueHit>(spacePoint);
      else if (spacePoint->getType() == VXD::SensorInfoBase::SVD) mcParticles = getMCParticles<SVDTrueHit>(spacePoint);
      nClusters += spacePoint->getNClustersAssigned(); // TODO: this function needs to be defined first!

      for (const std::pair<int, double>& anID : mcParticles) {
        B2DEBUG(999, "MCParticle " << anID.first << " has " << anID.second << " Clusters in this SpacePoint");
        mcPartMap[anID.first] += anID.second;
      }
      B2DEBUG(999, "Container contains " << nClusters << " Clusters up to now");
    }

    B2DEBUG(999, "MCParticle Ids (keys) and Clusters related to it (values) for this container: " + printMap(mcPartMap));

    std::vector<int> mcIds = getUniqueKeys(mcPartMap);
    for (int id : mcIds) {
      double purity = mcPartMap[id] / double(nClusters);
      B2DEBUG(999, "MCParticle " << id << " has purity " << purity << " in container " << container->getArrayIndex() <<
              " from Array " << container->getArrayName());
      purities.push_back(std::make_pair(id, purity));
    }

    return purities;
  }

  /**
   * function that takes any container holding SpacePoints that provide a getHits() function.
   * @return vector of pairs where .first is an McId and .second is the purity of this McId
   *
   * The purities are calculated via (# Clusters associated with MCId) / (# Clusters in all SpacePoints of container)
   * Clusters that cannot be associated with a MCParticle are put into MCId -1
   *
   * Overload function taking references for use in C++11 for loops
   */
  template<typename SPContainer>
  std::vector<std::pair<int, double> > calculatePurity(const SPContainer& container)
  {
    return calculatePurity(&container);
  }

}

