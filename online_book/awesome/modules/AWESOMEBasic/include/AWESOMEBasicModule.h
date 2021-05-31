/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *               Giacomo De Pietro                                        *
 *               Umberto Tamponi                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* AWESOME headers. */
#include <online_book/awesome/dataobjects/AWESOMESimHit.h>

/* Belle2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>

/* C++ headers. */
#include <string>
#include <vector>

namespace Belle2::AWESOME {

  /**
   * The AWESOME basic module.
   *
   * This is an almost empty module which just illustrates how to define
   * parameters and use the datastore.
   */
  class AWESOMEBasicModule : public Module {

  public:

    /**
     * Default constructor.
     */
    AWESOMEBasicModule();

    /**
     * Initialize the module.
     */
    void initialize() override;

    /**
     * Called for each begin of run.
     */
    void beginRun() override;

    /**
     * Called for each event.
     */
    void event() override;

    /**
     * Called for each end of run.
     */
    void endRun() override;

    /**
     * Called on termination.
     */
    void terminate() override;

  private:

    /** Useless variable showing how to create integer parameters. */
    int m_intParameter;

    /** Useless variable showing how to create double parameters. */
    double m_doubleParameter;

    /** Useless variable showing how to create string parameters. */
    std::string m_stringParameter;

    /** Useless variable showing how to create array parameters. */
    std::vector<double> m_doubleListParameter;

    /** MC particles. */
    StoreArray<MCParticle> m_MCParticles;

    /** AWESOME simulated hits. */
    StoreArray<AWESOMESimHit> m_SimHits;

  };

}
