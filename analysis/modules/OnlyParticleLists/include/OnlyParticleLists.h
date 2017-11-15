/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <framework/core/Module.h>

namespace Belle2 {
  /**
   * foo
   */
  class OnlyParticleListsModule : public Module {

  public:
    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    OnlyParticleListsModule() {}
    /** naah */
    virtual void initialize();
  };
}
