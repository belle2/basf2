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

    virtual void event() override;
  };
}
