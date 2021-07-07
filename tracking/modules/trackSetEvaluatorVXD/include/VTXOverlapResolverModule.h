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

  class VTXOverlapResolverModule : public Module {

  public:
    /** Constructor of the module. */
    VTXOverlapResolverModule();

    /** State requirements of SpacePointTrackCands, VTXClusters, registers OverlapNetworks.*/
    void initialize() override final;

    /** Checks for overlaps and fills the OverlapNetwork. */
    void event() override final;

  protected:
    /** Stay connected to the DataStore for timing improvement. */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;
    std::string m_nameSpacePointTrackCands; ///<Name of array of SpacePointTrackCand.

    /** Stay connected to the DataStore for timing improvement. */
    StoreArray<VTXCluster> m_vtxClusters;
    std::string m_nameVTXClusters; ///<Name of array of VTXCluster.

    /** Strategy used to resolve overlaps. */
    std::string m_resolveMethod;

    /** Estimated amount of active candidates used to reserve enough space. */
    unsigned short m_estimatedActiveCandidates = 1000;

    /** Minimum of activityState of candidate required to be accepted by the algorithm. */
    float m_minActivityState = 0.3;
  };
}
