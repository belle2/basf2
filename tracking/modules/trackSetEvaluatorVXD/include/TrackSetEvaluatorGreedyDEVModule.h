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

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapNetwork.h>
#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

namespace Belle2 {

  /** The Greedy algoritm Track-set-evaluator.
   *
   *  Expects SpacePointTrackCandidates and an OverlapNetwork.
   *  Selects a subset of non-overlapping Track candidates
   *  determined using the Greedy algorithm, which takes the highest quality candidate
   *  and then simply kills of all conflicting tracks.
   */
  class TrackSetEvaluatorGreedyDEVModule : public Module {
  public:
    /** Constructor of the module. */
    TrackSetEvaluatorGreedyDEVModule();

    /** Requires SpacePointTrackCands, OverlapNetworks. */
    void initialize() override final
    {
      m_spacePointTrackCands.isRequired(m_nameSpacePointTrackCands);
      m_overlapNetworks.isRequired(m_nameOverlapNetworks);
    }

    /** Application of the algorithm. */
    void event() override final;

  protected:
    /** the storeArray for SpacePointTrackCands as member, is faster than recreating link for each event */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;
    std::string m_nameSpacePointTrackCands; ///<Name of array of SpacePointTrackCand.

    /** access to tcNetwork, which will be produced by this module */
    StoreArray<OverlapNetwork> m_overlapNetworks;
    /** name of the overlap networks */
    std::string m_nameOverlapNetworks;
  };
}
