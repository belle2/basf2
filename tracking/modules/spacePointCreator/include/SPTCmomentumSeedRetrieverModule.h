/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <string>


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
    void initialize() override
    {
      InitializeCounters();
      m_spacePointTrackCands.isRequired(m_PARAMtcArrayName);
    }


    /**
     * Prints a header for each new run.
     */
    void beginRun() override;


    /** Applies the circleFit at given sets of TCs. */
    void event() override;


    /** Prints a footer for each run which ended. */
    void endRun() override;


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

    // member variables

    /** Bz component of the magnetic field, will be updated runwise. */
    double m_bFieldZ = 0.;

    /** knows current event number. */
    unsigned int m_eventCounter = 0;

    /** total number of TCs found in SpacePointTrackCand-container. */
    unsigned int m_nTCsTotal = 0;

    /** the storeArray for SpacePointTrackCands as member, is faster than recreating link for each event. */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** the QualityEstimator used to determine momentum seed. */
    QualityEstimatorRiemannHelixFit m_estimator;

  private:
  };
}
