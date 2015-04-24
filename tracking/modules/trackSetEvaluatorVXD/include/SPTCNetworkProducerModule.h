/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/spacePointCreation/sptcNetwork/SpTcNetwork.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/core/Module.h>
#include <string>
#include <vector>


namespace Belle2 {

  /** The Greedy algoritm Track-set-evaluator.
   *
   * This module expects a container of SpacePointTrackCandidates and builds a network of SpacePointTrackCands.
   * That network keeps track of their connections (overlaps) during evaluation phase of other modules (e.g. TrackSetEvaluatorGreedyModule).
   */
  class SPTCNetworkProducerModule : public Module {

  public:


    /**
     * Constructor of the module.
     */
    SPTCNetworkProducerModule();


    /** Initializes the Module.
     */
    virtual void initialize()
    {
      InitializeCounters();
      m_spacePointTrackCands.isRequired(m_PARAMtcArrayName);
      m_tcNetwork.registerInDataStore(m_PARAMtcNetworkName, DataStore::c_DontWriteOut);
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


  protected:

    // module parameters


    /** sets the name of expected StoreArray with SpacePointTrackCand in it. */
    std::string m_PARAMtcArrayName;


    /** sets the name of the StoreObjPtr used for storing a TC network. */
    std::string m_PARAMtcNetworkName;


    /** if true, overlaps are checked via SpacePoints. If false, overlaps are checked via clusters */
    bool m_PARAMcheckSPsInsteadOfClusters;


    // member variables


    /** knows current event number */
    unsigned int m_eventCounter;


    /** total number of TCs found in SpacePointTrackCand-container */
    unsigned int m_nTCsTotal;


    /** total number of TCs == nodes found in SpTcNetwork-container */
    unsigned int m_nNodesNetwork;


    /** total number of competitors == links found in SpTcNetwork-container */
    unsigned int m_nLinksNetwork;


    /** the storeArray for SpacePointTrackCands as member, is faster than recreating link for each event */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;


    /** access to tcNetwork, which will be produced by this module */
    StoreObjPtr<SpTcNetwork> m_tcNetwork;


  private:

  };
}
