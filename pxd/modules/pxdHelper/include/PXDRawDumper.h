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

    class PXDRawDumperModule: public Module {
    private:
      StoreArray<RawPXD> m_storeRaw;

      bool getTrigNr(RawPXD& px, unsigned int& innerDHH, unsigned int& outerHLT);
      bool unpack_dhc_frame(void* data, unsigned int& innerDHH, unsigned int& outerHLT);

    public:
      void initialize(void);
      void event(void);
    };
  }
}
