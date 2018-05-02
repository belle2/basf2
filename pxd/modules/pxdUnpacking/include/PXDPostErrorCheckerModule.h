/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDDAQStatus.h>

#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  namespace PXD {
    /** The PXD DAQ Post Unpacking Error Check.
     *
     * does a few error checks which cannot be done during unpacking
     */
    class PXDPostErrorCheckerModule : public Module {

    public:

      /** Constructor defining the parameters */
      PXDPostErrorCheckerModule();

    private:
      void initialize() override final;

      void event() override final;

    private:

      /** Input array for DAQ Status. */
      StoreObjPtr<PXDDAQStatus> m_storeDAQEvtStats;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
