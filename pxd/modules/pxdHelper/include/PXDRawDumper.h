/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawPXD.h>

namespace Belle2 {

  namespace PXD {

    /** Dump Raw PXD/ ONSEN event data */
    class PXDRawDumperModule: public Module {
    private:
      StoreArray<RawPXD> m_storeRaw; ///< Store array of RawPXDs

      /** Get the trigger number */
      bool getTrigNr(RawPXD& px, unsigned int& innerDHH, unsigned int& outerHLT);

      /** Unpack the DHC frame */
      bool unpack_dhc_frame(void* data, unsigned int& innerDHH, unsigned int& outerHLT);

    public:
      /** Initialize */
      void initialize(void) override;
      /** Event */
      void event(void) override;
    };
  }
}
