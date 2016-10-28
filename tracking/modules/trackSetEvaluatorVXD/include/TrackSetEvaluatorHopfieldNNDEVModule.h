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
// #include <tracking/spacePointCreation/sptcNetwork/SpTcNetwork.h>
#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapNetwork.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/Module.h>
#include <string>
#include <vector>

#include <TRandom.h>


namespace Belle2 {

  /** The Greedy algoritm Track-set-evaluator.
   *
   * This module expects a container of SpacePointTrackCandidates and selects a subset of non-overlapping TCs determined using a neural network of Hopfield type.
   */
  class TrackSetEvaluatorHopfieldNNDEVModule : public Module {

  public:

    /** ************************************** Structs to help simplify the process ************************************************ */

    /** small container to carry all the info needed for Hopfield algorithm*/
    struct TcInfo4Hopfield {
      /** new ID the TC used for the reduced network only. Reduced network only contains overlapping tcs for faster algorithm execution */
      unsigned int reducedID;

      /** the corresponding iD of the TC in the real network, allows access to TC and competitor */
      unsigned int realID;

      /** carries iDs for competitors, after creation the reduced iDs can be found here */
      std::vector<unsigned short>competitorsIDs;

      /** stores the neuron value */
      double neuronValue = 0.;

      /** quality indicator (higher is better) */
      double qi;
    };


    /** Wrap TRandom to be useable as a uniform random number generator with std
     * algorithms like std::shuffle. */
    struct TRandomWrapper {
      /** define the result type to be a normal unsigned int */
      typedef unsigned int result_type;

      /** minimum value returned by the random number generator */
      static constexpr result_type min() { return 0; }

      /** maximum value returned by the random number generator */
      static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

      /** return a random value in the range [min(), max()]. */
      result_type operator()() { return gRandom->Integer(max()); }
    };


    /** ************************************** Member functions ************************************************ */

    /** Constructor of the module. */
    TrackSetEvaluatorHopfieldNNDEVModule();


    /** Initializes the Module. */
    virtual void initialize() override final
    {
      m_spacePointTrackCands.isRequired(m_PARAMtcArrayName);
      m_overlapNetworks.isRequired(m_PARAMtcNetworkName);
    }


    /** set up for the begin of the run. */
    virtual void beginRun() override final { InitializeCounters(); }


    /** Applies the Hopfield neural network algorithm at given sets of TCs. */
    virtual void event() override final;


    /** End run clean up. */
    virtual void endRun() override final;


    /** initialize variables to avoid nondeterministic behavior TODO check if still needed! */
    void InitializeCounters();


    /** for that easy situation we don't need sophisticated algorithms for finding the best subset. neuron value of TC with higher QI is set to 1, while the other is set to 0. */
    void tcDuel(std::vector<TcInfo4Hopfield>& overlappingTCs);


    /** typically of a set of TCs some are overlapping and some aren't.
     * To speed up the HNN, only overlapping TCs are stored in the vector of overlapping TCs.
     */
    std::vector<TcInfo4Hopfield> reduceOverlappingNetwork();


    /** if reduceTCSetBeforeHNN is false, the data is simply converted to be readable for the HNN and no non-overlapping TCs are filtered. */
    std::vector<TcInfo4Hopfield> convertOverlappingNetwork();

    /** Hopfield neural network function, returns true if it was successful.
     *
     * Algorithm:
     * Neural network of Hopfield type as described in:
     * R. Frühwirth, “Selection of optimal subsets of tracks with a feedback neural network,”
     * C omput.Phys.Commun., vol. 78, pp. 23–28, 1993.*
     */
    bool doHopfield(std::vector<TcInfo4Hopfield>& tcs, double omega = 0.5);


  protected:

    /** ************************************** Module Parameters ************************************************ */

    /** sets the name of expected StoreArray with SpacePointTrackCand in it. */
    std::string m_PARAMtcArrayName;

    /** sets the name of the StoreObjPtr used for storing a TC network. */
    std::string m_PARAMtcNetworkName;

    /** f true, only overlapping TCs are considered by the HNN, if false all TCs are considered, including non-overlapping ones */
    bool m_PARAMreduceTCSetBeforeHNN = false;


    /** ************************************** Member variables ************************************************ */

    /** the storeArray for SpacePointTrackCands as member, is faster than recreating link for each event. */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** access to tcNetwork, which will be produced by this module. */
    StoreArray<OverlapNetwork> m_overlapNetworks;

    /** knows current event number. */
    unsigned int m_eventCounter;

    /** total number of TCs evaluated so far. */
    unsigned int m_nTCsTotal;

    /** number of clean TCs in total spTC-network before cleaning it. */
    unsigned int m_nTCsCleanAtStart;

    /** number of overlapping TCs so far. */
    unsigned int m_nTCsOverlapping;

    /** number of TCs found for final sets of non-overlapping TCs so far. */
    unsigned int m_nFinalTCs;

    /** number of TCs rejected for final sets of non-overlapping TCs so far. */
    unsigned int m_nRejectedTCs;

    /** counts number of times when Hopfield was not able to clean overlaps. */
    unsigned int m_nHopfieldFails;

  private:
  };
}
