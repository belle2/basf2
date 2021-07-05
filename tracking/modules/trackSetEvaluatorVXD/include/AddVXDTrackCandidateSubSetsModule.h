/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/tcTools/SpacePointTrackCandCreator.h>
#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

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
