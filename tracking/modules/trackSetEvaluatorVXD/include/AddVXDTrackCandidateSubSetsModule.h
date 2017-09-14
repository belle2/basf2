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

    void addSubCandidates(int iCand);

    /** Name of input StoreArray containing SpacePointTrackCands */
    std::string m_nameSpacePointTrackCands;


    // member variables

    /** tool for creating SPTCs, fills storeArray directly */
    SpacePointTrackCandCreator<StoreArray<Belle2::SpacePointTrackCand>> m_sptcCreator;

    /** StoreArray for input SpacePointTrackCands*/
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;
  };
}
