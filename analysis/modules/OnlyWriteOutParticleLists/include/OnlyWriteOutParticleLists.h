/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: James Kahn, Martin Ritter                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <framework/core/Module.h>

namespace Belle2 {
  /**
   * Marks all objects in DataStore except those of type ParticleList as WrtieOut=False.
   * Intedend to run before outputting an index file to remove unecessary arrays.
   */
  class OnlyWriteOutParticleListsModule : public Module {

  public:
    /**
     * Constructor
     */
    OnlyWriteOutParticleListsModule();
    /** Initialises the module.
    * Where the datastore entries are marked for WriteOut.
    */
    virtual void initialize();
  };
}
