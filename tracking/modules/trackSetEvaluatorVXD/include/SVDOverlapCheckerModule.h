/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapNetwork.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <vector>
#include <string>

namespace Belle2 {

  /** Checks overlap of SpacePointTrackCandidates, e.g. multiple usage of the same 1D Cluster.
   *
   *  Expects StoreArray of SpacePointTrackCandidates;
   *  Produces OverlapNetwork, that can be asked, with which other ones a track candidate overlaps;
   *
   *  The algorithm idea is the following:<br>
   *  - Loop over all SpacePointTrackCandidates<br>
   *  - Loop over all SpacePoints of each candidate<br>
   *  - Fill for 1D SVD clusters a matrix[ClusterID][TrackIndex] <br>
   *  - Fill the OverlapNetwork (which really is an overlap matrix)
   */
  class SVDOverlapCheckerModule : public Module {

  public:
    /** Constructor of the module. */
    SVDOverlapCheckerModule();

    /** State requirements of SpacePointTrackCands, SVDClusters, registers OverlapNetworks.*/
    void initialize() override final
    {
      m_spacePointTrackCands.isRequired(m_nameSpacePointTrackCands);
      m_svdClusters.isRequired();

      m_overlapNetwork.registerInDataStore(DataStore::c_DontWriteOut);
    }

    /** Checks for overlaps and fills the OverlapNetwork. */
    void event() override final;

  protected:
    /** Stay connected to the DataStore for timing improvement. */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;
    std::string m_nameSpacePointTrackCands; ///<Name of array of SpacePointTrackCand.

    /** Stay connected to the DataStore for timing improvement. */
    StoreArray<SVDCluster> m_svdClusters;

    /** Stay connected to the DataStore for timing improvement.
     *
     *  Kept in a StoreArray instead of ...Object,
     *  as we might want to store one overlapNetwork per pass.
     */
    StoreArray<OverlapNetwork> m_overlapNetwork;
  };
}
