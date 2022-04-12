/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <string>

namespace Belle2 {

  /**
   * SVD overlap resolver module.
   */
  class SVDOverlapResolverModule : public Module {

  public:
    /** Constructor of the module. */
    SVDOverlapResolverModule();

    /** State requirements of SpacePointTrackCands, SVDClusters, registers OverlapNetworks.*/
    void initialize() override final;

    /** Checks for overlaps and fills the OverlapNetwork. */
    void event() override final;

  protected:
    /** Stay connected to the DataStore for timing improvement. */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;
    std::string m_nameSpacePointTrackCands; ///<Name of array of SpacePointTrackCand.

    /** Stay connected to the DataStore for timing improvement. */
    StoreArray<SVDCluster> m_svdClusters;
    std::string m_nameSVDClusters; ///<Name of array of SVDCluster.

    /** Strategy used to resolve overlaps. */
    std::string m_resolveMethod;

    /** Estimated amount of active candidates used to reserve enough space. */
    unsigned short m_estimatedActiveCandidates = 1000;

    /** Minimum of activityState of candidate required to be accepted by the algorithm. */
    float m_minActivityState = 0.3;
  };
}
