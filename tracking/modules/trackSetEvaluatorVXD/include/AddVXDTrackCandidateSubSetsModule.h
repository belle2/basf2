/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/tcTools/SpacePointTrackCandCreator.h>
#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>



namespace Belle2 {

  /** Module that creates additional candidates that each miss a different SpacePoint.
   *
   *  Expects SpacePointTrackCandidates.
   *  Creates new candidates by removing single SpacePoints
   */
  class AddVXDTrackCandidateSubSetsModule : public Module {
  public:
    /** Constructor of the module. */
    AddVXDTrackCandidateSubSetsModule();

    /** Requires SpacePointTrackCands. */
    void initialize() override final;

    /** event loop. */
    void event() override final;

  protected:

    /** Actually creates the new SPTCs by removing single SPs from the SPTC with the provided StoreArray index. */
    void addSubCandidates(int iCand);

    /** Name of input StoreArray containing SpacePointTrackCands */
    std::string m_nameSpacePointTrackCands = std::string("");

    /// member variables
    /** tool for creating SPTCs, fills storeArray directly */
    SpacePointTrackCandCreator<StoreArray<Belle2::SpacePointTrackCand>> m_sptcCreator;

    /** StoreArray for input SpacePointTrackCands */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** Minimal number of SPs of the original SPTC */
    int m_minOriginalSpacePoints = 4;
  };
}
