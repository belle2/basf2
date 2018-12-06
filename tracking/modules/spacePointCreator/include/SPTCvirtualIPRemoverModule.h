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

  /** A module for checking and removing the virtual IP if wanted. */
  class SPTCvirtualIPRemoverModule : public Module {

  public:


    /**
     * Constructor of the module.
     */
    SPTCvirtualIPRemoverModule();


    /** Initializes the Module.
     */
    void initialize() override
    {
      InitializeCounters();
      m_spacePointTrackCands.isRequired(m_PARAMtcArrayName);
    }


    /**
     * Prints a header for each new run.
     */
    void beginRun() override
    {
      InitializeCounters();
    }


    /** Applies the circleFit at given sets of TCs. */
    void event() override;


    /** Prints a footer for each run which ended. */
    void endRun() override;


    /** initialize variables to avoid nondeterministic behavior */
    void InitializeCounters()
    {
      m_eventCounter = 0;
      m_nTCsTotal = 0;
      m_nVIPsTotal = 0;
      m_nVIPsRemoved = 0;
    }


  protected:

    // module parameters


    /** sets the name of expected StoreArray with SpacePointTrackCand in it. */
    std::string m_PARAMtcArrayName;

    /** if true, no vIP is removed, but only nVIPs are counted. */
    bool m_PARAMdoCheckOnly;

    /** If you want to keep the vIP only for short TCs, then set this value to the number of hits a TC is maximally allowed to have to not loose its vIP.
     *
     * (number of hits without counting the vIP). */
    unsigned int m_PARAMmaxTCLengthForVIPKeeping;


    // member variables


    /** knows current event number. */
    unsigned int m_eventCounter;

    /** total number of TCs found in SpacePointTrackCand-container. */
    unsigned int m_nTCsTotal;

    /** total number virtualIPs found. */
    unsigned int m_nVIPsTotal;

    /** total number of TCs removed from TCs. */
    unsigned int m_nVIPsRemoved;

    /** the storeArray for SpacePointTrackCands as member, is faster than recreating link for each event. */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

  private:
  };
}
