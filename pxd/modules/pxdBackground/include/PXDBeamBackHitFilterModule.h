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
