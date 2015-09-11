/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <framework/core/Module.h>

namespace Belle2 {

  class WireHitTopologyPreparerModule : public Module {

  public:
    WireHitTopologyPreparerModule() : Module()
    {
      setDescription("Combine the CDCHits from the DataStore with the geometry information to have them both at hand in the CDC tracking modules. "
                     "Also set all CDCWireHits as unused.");
    }

    /// Create the StoreObject.
    void initialize();

    /// Load the hits from the StoreArray.
    void event();
  };

}
