/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  /**
   * Imports Clusters of the silicon detectors and converts them to spacePoints.
   *
   */
  class SpacePointCreatorModule : public Module {

  public:

    /** Constructor */
    SpacePointCreatorModule();

    /** Init the module.
    *
    * prepares all store- and relationArrays
    */
    virtual void initialize();
    /** Show progress */
    virtual void event();
    /** Don't break the terminal */
    virtual void terminate();

  protected:

    // Data members
    std::string m_pxdClustersName; /**< PXDCluster collection name */
    std::string m_svdClustersName; /**< SVDCluster collection name */
    std::string m_spacePointsName; /**< SpacePoints collection name */
    std::string m_relSpacePointsPXDClustersName; /**< SpacePoints <-> PXDClusters relation name */
    std::string m_relSpacePointsSVDClustersName; /**< SpacePoints <-> SVDClusters relation name */

    //counters for testing
    unsigned int m_TESTERPXDClusterCtr; /**< counts total number of PXDClusters occured */
    unsigned int m_TESTERSVDClusterCtr; /**< counts total number of SVDCluster occured */
    unsigned int m_TESTERSpacePointCtr; /**< counts total number of SpacePoints occured */
  };
} // end namespace Belle2
