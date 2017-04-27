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


namespace Belle2 {

  /** Module that selects a subset with a fixed minimum QualityIndex out of all SpacePointTrackCandidates.
   *
   *  Expects SpacePointTrackCandidates.
   *  Selects all candidates passing a qualityIndex requirement
   *  Either deactivates the remaining candidates
   *  or fills a new StoreArray with the selected candidates.
   */
  class VXDTrackCandidatesQualityIndexCutterModule : public Module {
  public:
    /** Constructor of the module. */
    VXDTrackCandidatesQualityIndexCutterModule();

    /** Requires SpacePointTrackCands. */
    void initialize() override final;

    /** Application of the cut. */
    void event() override final;

  protected:

    void deactivateCandidates();

    void copyCandidates();

    // parameters

    float m_minRequiredQuality;

    /** Name of input StoreArray containing SpacePointTrackCands */
    std::string m_nameSpacePointTrackCands;

    /** If True copy selected SpacePoints to new StoreArray,
     * If False deactivate remaining SpacePoints.
     */
    bool m_createNewStoreArray;

    /** Name of optional output StoreArray containing SpacePointTrackCands */
    std::string m_newNameSpacePointTrackCands;


    // member variables

    /** StoreArray for input SpacePointTrackCands*/
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** StoreArray for optional output SpacePointTrackCands*/
    StoreArray<SpacePointTrackCand> m_newSpacePointTrackCands;
  };
}
