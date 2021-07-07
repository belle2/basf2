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
   * Marks all objects in DataStore except those of type ParticleList as WrtieOut=False.
   * Intended to run before outputting an index file to remove unnecessary arrays.
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
    virtual void initialize() override;
  };
}
