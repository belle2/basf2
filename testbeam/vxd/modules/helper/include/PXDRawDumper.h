#ifndef PXDRAWDUMPERMODULE_H_
#define PXDRAWDUMPERMODULE_H_

#include <framework/core/Module.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

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
#endif