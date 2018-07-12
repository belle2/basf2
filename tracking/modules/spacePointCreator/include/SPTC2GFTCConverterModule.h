/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

// #include <framework/datastore/RelationVector.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <genfit/TrackCand.h>

namespace Belle2 {
  /**
   * Module for converting SpacePointTrackCands to genfit::SpacePointTrackCands.
   * Module simply takes every SpacePoint from the SpacePointTrackCand and gets all the Clusters related to it. From the indices of the Cluster in its StoreArray a genfit::TrackCandHit can be generated which is then added to the genfit::TrackCand. If further information is present (e.g. PDG code, charge estimate, etc.) this information is also added to the genfit::TrackCand.
   */
  class SPTC2GFTCConverterModule : public Module {

  public:

    SPTC2GFTCConverterModule(); /**< Constructor */

    void initialize() override; /**< initialize module */

    void event() override; /**< event: convert SpacePointTrackCand to genfit::TrackCand */

    void terminate() override; /**< terminate: some summary information on the processed events */

  protected:

    std::string m_SPTCName; /**< SpacePointTrackCand collection name */

    std::string m_genfitTCName; /**< genfit::TrackCand collection name */

    std::string m_PXDClustersName; /**< PXD Clusters collection name */

    std::string m_SVDClustersName; /**< SVD Clusters collection name */

    // some counters for summary output
    unsigned int m_SpacePointTCCtr; /**< Counter for SpacePointTrackCands presented to the module */

    unsigned int m_genfitTCCtr; /**< Counter for genfit::TrackCands that were actually created by the module */

    unsigned int m_skippedSPsCtr; /**< Counter for SpacePoints that were not converted */

    void initializeCounters(); /**< reset counters to 0 to avoid indeterministic behaviour */

    /** get all related Clusters to a SpacePoint
     * @param clusterNames the Name of the StoreArray where Clusters shall be looked for
     */
    template<typename ClusterType>
    std::vector<int> getRelatedClusters(const Belle2::SpacePoint* spacePoint, const std::string& clusterNames = "ALL");

    BELLE2_DEFINE_EXCEPTION(ClusterNotFound,
                            "No related Cluster to a SpacePoint was found."); /**< Exception thrown, when no related Cluster can be found for a SpacePoint */
  };
}