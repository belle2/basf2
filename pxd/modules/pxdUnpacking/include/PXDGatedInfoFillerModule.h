/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      /** Initialize */
      void initialize() override final;

      /** Event */
      void event() override final;

    private:
      /** Name of input array for DAQ Status. */
      std::string m_GatedModeInfoName;
      /** Input array for DAQ Status. */
      StoreObjPtr<PXDGatedModeInfo> m_storeGatedModeInfo;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
