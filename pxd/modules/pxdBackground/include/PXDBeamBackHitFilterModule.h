/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {
  namespace PXD {

    /** The PXDBeamBackHitFilter module.
     *
     * This module removes other than PXD-related BeamBackHits from the BeamBackHits StoreArray.
     */
    class PXDBeamBackHitFilterModule : public Module {

    public:
      /** Constructor defining the parameters */
      PXDBeamBackHitFilterModule();

      /** Initialize the module */
      virtual void initialize() override;
      /** do the sorting */
      virtual void event() override;

    private:

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
