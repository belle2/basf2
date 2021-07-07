/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/MCInitialParticles.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dbobjects/BeamParameters.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>

/* Belle headers. */
#include <belle_legacy/panther/panther_group.h>

namespace Belle2 {

  /**
   * KLM digitization module.
   */
  class BelleMCOutputModule : public Module {

  public:

    /**
     * Constructor.
     */
    BelleMCOutputModule();

    /**
     * Destructor
     */
    virtual ~BelleMCOutputModule();

    /**
     * Initializer.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     */
    virtual void beginRun() override;

    /**
     * This method is called for each event.
     */
    virtual void event() override;

    /**
     * This method is called if the current run ends.
     */
    virtual void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    virtual void terminate() override;

  private:

    /**
     * Add particle to graph. Daughters of particles that are stable in basf
     * are removed.
     * @param[in]     particle Particle.
     * @param[in]     mother   Mother particle
     */
    void addParticle(const MCParticle* particle,
                     MCParticleGraph::GraphParticle* mother);

    /** Output file name. */
    std::string m_OutputFileName;

    /** Decay K_S0 in generator. */
    bool m_DecayKsInGenerator;

    /** Decay Lambda in generator. */
    bool m_DecayLambdaInGenerator;

    /** MC particles. */
    StoreArray<MCParticle> m_MCParticles;

    /** Initial particles. */
    StoreObjPtr<MCInitialParticles> m_MCInitialParticles;

    /** Event metadata. */
    StoreObjPtr<EventMetaData> m_EventMetaData;

    /** Beam parameters. */
    DBObjPtr<BeamParameters> m_BeamParameters;

    /** MC particle graph. */
    MCParticleGraph m_MCParticleGraph;

    /** Belle file input-output handler. */
    Belle::Panther_FileIO* m_BelleFile = nullptr;

  };

}
