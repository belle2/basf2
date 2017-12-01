/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>


namespace Belle2 {
  /**
   * Start an interactive python shell in each call of event().
   *
   * Also imports the ROOT.Belle2 namespace for convenience, allowing you
   * to use Belle2.PyStoreArray etc. directly.
   */
  class InteractiveModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description, the properties and the parameters of the module.
     */
    InteractiveModule();

    ~InteractiveModule();

    virtual void event();
  };
}
