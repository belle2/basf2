/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <generators/evtgen/EvtGenInterface.h>
#include <generators/utilities/InitialParticleGeneration.h>
#include <mdst/dataobjects/MCParticleGraph.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  /**
   * This module decays unstable particles using EvtGen.
   */
  class EvtGenDecayModule : public Module {

  public:

    /**
     * Constructor.
     */
    EvtGenDecayModule();

    /**
     * Destructor.
     */
    virtual ~EvtGenDecayModule();

    /**
     * Initialization.
     */
    virtual void initialize() override;

    /**
     * This method is called when entering a new run.
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
     * Generator initialization.
     */
    void initializeGenerator();

    /** Decay file. */
    std::string m_DecFile;

    /** User decay file. */
    std::string m_UserDecFile;

    /** MCParticle collection name. */
    std::string m_MCParticleColName;

    /** MCParticle graph. */
    MCParticleGraph m_Graph;

    /** Evtgen interface. */
    EvtGenInterface m_EvtGenInterface;

    /** Whether the generator is initialized or not. */
    bool m_Initialized;

    /** Monte-Carlo codes of particles that can be decayed by EvtGen. */
    std::set<int> m_DecayableParticles;

    /** Beam parameters. */
    DBObjPtr<BeamParameters> m_BeamParameters;

  };

}


