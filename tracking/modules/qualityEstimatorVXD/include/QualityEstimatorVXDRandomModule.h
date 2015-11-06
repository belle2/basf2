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
#include <framework/datastore/StoreArray.h>

#include <framework/core/Module.h>
#include <string>
#include <vector>


namespace Belle2 {

  /** The quality estimator module for SpacePointTrackCandidates using random values.
   *
   * This module is to be used for comparison and testing purposes.
   */
  class QualityEstimatorVXDRandomModule : public Module {

  public:


    /**
     * Constructor of the module.
     */
    QualityEstimatorVXDRandomModule();


    /** Initializes the Module.
     */
    virtual void initialize()
    {
      InitializeCounters();
      m_spacePointTrackCands.isRequired(m_PARAMtcArrayName);
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


    bool m_PARAMuseTimeSeedAsQI; /**< JKL - WARNING evil hack: uses an ugly workaround to be able to use realistically determined quality indicators */


    // member variables


    /** knows current event number. */
    unsigned int m_eventCounter;


    /** total number of TCs found in SpacePointTrackCand-container. */
    unsigned int m_nTCsTotal;


    /** the storeArray for SpacePointTrackCands as member, is faster than recreating link for each event. */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

  private:
  };
}
