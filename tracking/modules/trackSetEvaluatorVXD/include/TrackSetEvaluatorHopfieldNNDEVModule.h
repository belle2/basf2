/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapNetwork.h>
#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapResolverNodeInfo.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/Module.h>

#include <string>
#include <vector>

#include <TRandom.h>


namespace Belle2 {

  /** The Hopfield algoritm Trackset Evaluator.
   *
   *  This module expects a container of SpacePointTrackCandidates (TCs) and selects a subset of non-overlapping
   *  TCs determined using a neural network of Hopfield type.
   *
   *  DESIGN DECISIONS:
   *  - The previous version has foreseen to first remove tracks without overlaps or subsamples,
   *    that don't overlap with different subsamples. However, the functionality isn't fully implemented and
   *    isn't used. To reduce the amount of dead code, we eliminated that option, but we can bring it back,
   *    if we think it is useful.
   *  - As well for the case of just two overlapping tracks, a fall back to the evaluation of all
   *    (2) combinations was foreseen. This is deleted as well.
   *  We recommend to first try with the generic case and use all possible optimizations there,
   *  before complicating the code structure again with the handling of special cases just for small
   *  speed improvements.
   */
  class TrackSetEvaluatorHopfieldNNDEVModule : public Module {

  public:
    /** Constructor of the module. */
    TrackSetEvaluatorHopfieldNNDEVModule();

    /** Statements of requirements (spacePointTrackCands & overlapNetworks). */
    virtual void initialize() override final
    {
      m_spacePointTrackCands.isRequired(m_PARAMtcArrayName);
      m_overlapNetworks.isRequired(m_PARAMtcNetworkName);
    }

    /** Applies the Hopfield neural network algorithm at given sets of SpacePointTrackCandidates. */
    void event() override final;

    /** Report on performance. */
    void endRun() override final;

  private:
    /** ************************************** Module Parameters ************************************************ */

    /** sets the name of expected StoreArray with SpacePointTrackCand in it. */
    std::string m_PARAMtcArrayName;

    /** sets the name of the StoreObjPtr used for storing a TC network. */
    std::string m_PARAMtcNetworkName;

    /** ************************************** Member variables ************************************************ */

    /** Minimum of activityState ("Neuron Value") required to be accepted by the algorithm. */
    float m_minActivityState;

    /** the storeArray for SpacePointTrackCands as member, is faster than recreating link for each event. */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** access to tcNetwork, which will be produced by this module. */
    StoreArray<OverlapNetwork> m_overlapNetworks;

    /** knows current event number. */
    unsigned int m_eventCounter       = 0;

    /** total number of TCs evaluated so far. */
    unsigned int m_nTCsTotal          = 0;

    /** number of TCs found for final sets of non-overlapping TCs so far. */
    unsigned int m_nFinalTCs          = 0;

    /** counts number of times when Hopfield was not able to clean overlaps. */
    unsigned int m_nHopfieldFails     = 0;
  };
}
