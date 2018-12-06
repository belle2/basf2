
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Johannes Rauch                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GenfitVisModule_H
#define GenfitVisModule_H

#include <framework/core/Module.h>
#include "genfit/EventDisplay.h"


namespace Belle2 {

  /**
   * Visualize genfit::Tracks using the genfit::EventDisplay.
   *
   *    *
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

#endif /* GenfitVisModule_H */
