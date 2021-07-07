/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include "genfit/EventDisplay.h"


namespace Belle2 {

  /**
   * Visualize genfit::Tracks using the genfit::EventDisplay.
   */
  class GenfitVisModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    GenfitVisModule();

    /** Initialize the EventDisplay */
    void initialize() override;

    /** Add genfit::Tracks to display. */
    void event() override;

    /** Open display. */
    void endRun() override;

    /** Close display. */
    void terminate() override;


  private:

    /** pointer to the genfit::EventDisplay which gets created in initialize() */
    genfit::EventDisplay* m_display = nullptr;
    /** if true, tracks which have been fitted and the fit converged will no be shown */
    bool m_onlyBadTracks;

  };
}
