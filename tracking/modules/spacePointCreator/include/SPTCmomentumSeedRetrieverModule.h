/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Metzner                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <framework/datastore/StoreArray.h>

#include <framework/core/Module.h>
#include <string>
//#include <vector>


namespace Belle2 {

  /** A module for creating momentum seeds for spacepoint track candidates. */
  class SPTCmomentumSeedRetrieverModule : public Module {

  public:


    /**
     * Constructor of the module.
     */
    SPTCmomentumSeedRetrieverModule();


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
    virtual void beginRun();


    /** Applies the circleFit at given sets of TCs. */
    virtual void event();


    /** Prints a footer for each run which ended. */
    virtual void endRun();


    /** initialize variables to avoid nondeterministic behavior */
    void InitializeCounters()
    {
      m_eventCounter = 0;
      m_nTCsTotal = 0;
    }

    /** Generates momentum seed at the position of innermost hit of given TC */
    bool createSPTCmomentumSeed(SpacePointTrackCand& aTC);

  protected:

    // module parameters

    /** sets the name of expected StoreArray with SpacePointTrackCand in it. */
    std::string m_PARAMtcArrayName;
    /** sets default PDG code for all track candidate in this module. */
    /** WARNING hardcoded values so far, should be passed by parameter! */
    int m_PARAMstdPDGCode = 211;


    // member variables

    /** Bz component of the magnetic field, will be updated runwise. */
    double m_bFieldZ = 0.;

    /** knows current event number. */
    unsigned int m_eventCounter;

    /** total number of TCs found in SpacePointTrackCand-container. */
    unsigned int m_nTCsTotal;

    /** the storeArray for SpacePointTrackCands as member, is faster than recreating link for each event. */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

  private:
  };
}
