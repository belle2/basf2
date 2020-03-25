//+
// File : PXDRawDumperModule.cc
// Description : Dump Raw PXD/ ONSEN event data
//
// Author : Bjoern Spruck
// Date : Updated on 20 - Dec - 2019
//-


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
      void initialize(void) override;
      void event(void) override;
    };
  }
}
