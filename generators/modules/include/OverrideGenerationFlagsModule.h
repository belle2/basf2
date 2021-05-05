/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
