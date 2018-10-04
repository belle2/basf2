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
//#include <pxd/dataobjects/PXDDAQStatus.h>
#include <pxd/dataobjects/PXDGatedModeInfo.h>

#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  namespace PXD {
    /** PXD Gates Mode infromation on readout gate basis
     *
     */
    class PXDGatedInfoFillerModule : public Module {

    public:

      /** Constructor defining the parameters */
      PXDGatedInfoFillerModule();

    private:
      void initialize() override final;

      void event() override final;

    private:
      std::string m_GatedModeInfoName;
      /** Input array for DAQ Status. */
      StoreObjPtr<PXDGatedModeInfo> m_storeGatedModeInfo;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
