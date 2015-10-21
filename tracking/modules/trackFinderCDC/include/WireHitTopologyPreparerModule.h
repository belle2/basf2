/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <framework/core/Module.h>

namespace Belle2 {

  /** Module to set up the cdcwire hits used in all CDC track finding algorithms (except Trasan).
   * This module combines the geometrical information and the raw hit information into CDC Wire Hits, which can be used from all modules after that.
   * The wire hits with all their flags (like taken or background) are saved in a object on the store array that is constructed in this module.
   * You can access the CDCWireHitTopology object like a singleton (which lives on the store array) and you do not need to worry about it.
   *
   * Please keep in mind that the taken flag is propagated to all modules. So if you set/unset the taken flag of one hit, this will be used by *all*
   * following modules. Also, in your own module, check for the taken flag/background flag and do not use already taken hits twice.
   */
  class WireHitTopologyPreparerModule : public Module {

  public:
    /// Construct the module. IOt has no parameters.
    WireHitTopologyPreparerModule() : Module()
    {
      setDescription("Combine the CDCHits from the DataStore with the geometry information to have them both at hand in the CDC tracking modules. "
                     "Also set all CDCWireHits as unused.");

      setPropertyFlags(c_ParallelProcessingCertified);
    }

    /// Create the StoreObject.
    void initialize();

    /// Load the hits from the StoreArray.
    void event();
  };

}
