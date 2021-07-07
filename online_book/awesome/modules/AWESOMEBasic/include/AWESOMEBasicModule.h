/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
