/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Constructs Tracks and ExtHits from MCParticles and TOPBarHits
   * Utility needed for testing and debugging of TOP reconstruction
   */
  class TOPMCTrackMakerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPMCTrackMakerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:


  };

} // Belle2 namespace
