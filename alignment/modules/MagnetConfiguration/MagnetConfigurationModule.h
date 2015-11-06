/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MAGNETCONFIGURATIONMODULE_H
#define MAGNETCONFIGURATIONMODULE_H

#include <framework/core/Module.h>


namespace Belle2 {
  /**
   * Stores configuration of magnet for each event (nothing if magnet ON. Special object is OFF)
   *
   *
   *
   */
  class MagnetConfigurationModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    MagnetConfigurationModule();

    /** Init */
    virtual void initialize();

    /** evvent */
    virtual void event();


  private:

  };
}

#endif /* MAGNETCONFIGURATIONMODULE_H */
