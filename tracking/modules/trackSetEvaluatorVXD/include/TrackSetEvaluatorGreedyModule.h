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
#include <tracking/spacePointCreation/sptcNetwork/SpTcNetwork.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/Module.h>
#include <string>
#include <vector>


namespace Belle2 {

  /** The Greedy algoritm Track-set-evaluator.
   *
   * This module expects a container of SpacePointTrackCandidates and selects a subset of non-overlapping TCs determined using the Greedy algorithm.
   */
  class TrackSetEvaluatorGreedyModule : public Module {

  public:


    /**
     * Constructor of the module.
     */
    TrackSetEvaluatorGreedyModule();


    /** Initializes the Module.
     */
    virtual void initialize()
    {
      m_spacePointTrackCands.isRequired(m_PARAMtcArrayName);
      m_tcNetwork.isRequired(m_PARAMtcNetworkName);
    }


    /**
     * Prints a header for each new run.
     */
    virtual void beginRun()
    {
      InitializeCounters();
    }


    /** Applies the Greedy algorithm at given sets of TCs. */
    virtual void event();


    /** Prints a footer for each run which ended. */
    virtual void endRun();


    /** initialize variables to avoid nondeterministic behavior */
    void InitializeCounters();


    /** checks track candidates for min and max QI-values */
    void checkMinMaxQI();


    /** for that easy situation we don't need sophisticated algorithms for finding the best subset... */
    template <typename TCCollectionType>
    void tcDuel(TCCollectionType& tcVector);


  protected:

    // parameters:
    /** if true, analysis data is written into a root file - standard is false */
    bool m_PARAMwriteToRoot;

    /** sets the name of expected StoreArray with SpacePointTrackCand in it. */
    std::string m_PARAMtcArrayName;

    /** sets the name of the StoreObjPtr used for storing a TC network. */
    std::string m_PARAMtcNetworkName;

    // member variables
    /** the storeArray for SpacePointTrackCands as member, is faster than recreating link for each event */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** access to tcNetwork, which will be produced by this module */
    StoreObjPtr<SpTcNetwork> m_tcNetwork;

    /** knows current event number */
    unsigned int m_eventCounter;

    /** total number of TCs evaluated so far */
    unsigned int m_nTCsTotal;

    /** number possible TC-combinations occured so far */
    unsigned int m_nTCsCompatible;

    /** number of overlapping TCs so far */
    unsigned int m_nTCsOverlapping;

    /** number of TCs found for final sets of non-overlapping TCs so far */
    unsigned int m_nFinalTCs;

    /** number of TCs rejected for final sets of non-overlapping TCs so far */
    unsigned int m_nRejectedTCs;

    /** sums up all QI-values of given TCs at start */
    double m_totalQI;

    /** sums up all QI-values of given TCs surviving the evaluation */
    double m_totalSurvivingQI;

    /** lowest quality indicator value found among TCs */
    double m_minQI;

    /** highests quality indicator value found among TCs */
    double m_maxQI;

    /** counts number of times when Greedy was not able to clean overlaps */
    unsigned int m_nGreedyFails;
  private:

  };
}
