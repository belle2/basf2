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
#include <framework/dbobjects/BeamParameters.h>

namespace Belle2 {

  /**
   * Ovverride generation flags.
   */
  class OverrideGenerationFlagsModule : public Module {

  public:

    /**
     * Constructor.
     */
    OverrideGenerationFlagsModule();

    /**
     * Destructor.
     */
    virtual ~OverrideGenerationFlagsModule();

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

    /** Beam parameters. */
    DBObjPtr<BeamParameters> m_BeamParameters;

    /** Smear energy when generating initial events. */
    bool m_SmearEnergy;

    /** Smear beam direction when generating initial events. */
    bool m_SmearDirection;

    /** Smear vertex position when generating initial events. */
    bool m_SmearVertex;

    /** Generate events in CMS, not lab system. */
    bool m_GenerateCMS;

  };
}
