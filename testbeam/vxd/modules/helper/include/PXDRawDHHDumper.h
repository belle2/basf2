#ifndef PXDRAWDHHDUMPERMODULE_H_
#define PXDRAWDHHDUMPERMODULE_H_

#include <framework/core/Module.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <testbeam/vxd/dataobjects/RawDHH.h>

namespace Belle2 {

  namespace PXD {

    class PXDRawDHHDumperModule: public Module {
    private:
      StoreArray<RawDHH> m_storeRaw;

      void endian_swapper(void* a, unsigned int len);
      bool getTrigNr(RawDHH& px, unsigned int& innerDHH, unsigned int& outerHLT);
      bool unpack_dhc_frame(void* data, unsigned int& innerDHH, unsigned int& outerHLT);

    public:
      void initialize(void);
      void event(void);
    };
  }
}
#endif
